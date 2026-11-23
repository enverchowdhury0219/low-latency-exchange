#include "exchange/market_maker.hpp"
#include "exchange/exchange.hpp"
#include <cmath>

namespace exchange
{

// half_spread is the param passed into constructor
// half_spread_ is member var stored in obj (the extra _ usually names private member vars)
MarketMaker::MarketMaker(
    Price half_spread,
    Price inventory_skew,
    std::int64_t inventory_unit,
    std::int64_t max_inventory
)
    // : here starts memver initializer list
    // so when making this MarketMaker obj, init its half_spread_ using param half_spread
    // this direct init helps to avoid later work
    : half_spread_(half_spread),
    inventory_skew_(inventory_skew),
    inventory_unit_(inventory_unit),
    max_inventory_(max_inventory)
// this is our constructor body (req even if empty), empty bc we did what we needed to in initializer list
{
}   

// read-only member function
Quote
MarketMaker::make_quote(Price fair_value) const
{
    Price adjusted_fair_value = fair_value;

    const Price inventory_adjustment =
    static_cast<Price>(
        (inventory_ * inventory_skew_) /
        inventory_unit_
    );

    adjusted_fair_value -=
        inventory_adjustment;

    // our Quote object
    return {
        adjusted_fair_value - half_spread_,
        adjusted_fair_value + half_spread_
    };
}

// to set inventory value
void
MarketMaker::set_inventory(
    std::int64_t inventory
)
{
    inventory_ = inventory;
}
// to retrieve inventory value
std::int64_t
MarketMaker::inventory() const
{
    return inventory_;
}

void
MarketMaker::on_fill(
    Side side,
    Price price,
    Quantity quantity
)
{
    const auto signed_quantity =
        static_cast<std::int64_t>(quantity);

    const auto notional =
        price * signed_quantity;

    if (side == Side::Buy)
    {
        inventory_ += signed_quantity;
        cash_ -= notional;
    }
    else
    {
        inventory_ -= signed_quantity;
        cash_ += notional;
    }
}

void
MarketMaker::set_active_orders(
    OrderId bid_order_id,
    Quantity bid_quantity,
    OrderId ask_order_id,
    Quantity ask_quantity
)
{
    active_bid_ = ActiveOrder{
        bid_order_id,
        bid_quantity
    };

    active_ask_ = ActiveOrder{
        ask_order_id,
        ask_quantity
    };
}

bool
MarketMaker::on_trade(
    const Trade& trade
)
{
    bool involved = false;

    if (
        active_bid_.has_value() &&
        trade.buy_order_id == active_bid_->id
    )
    {
        on_fill(
            Side::Buy,
            trade.price,
            trade.quantity
        );

        if (
            trade.quantity >=
            active_bid_->remaining_quantity
        )
        {
            active_bid_.reset();
        }
        else
        {
            active_bid_->remaining_quantity -=
                trade.quantity;
        }

        involved = true;
    }

    if (
        active_ask_.has_value() &&
        trade.sell_order_id == active_ask_->id
    )
    {
        on_fill(
            Side::Sell,
            trade.price,
            trade.quantity
        );

        if (
            trade.quantity >=
            active_ask_->remaining_quantity
        )
        {
            active_ask_.reset();
        }
        else
        {
            active_ask_->remaining_quantity -=
                trade.quantity;
        }

        involved = true;
    }

    return involved;
}

void
MarketMaker::on_events(
    const std::vector<ExchangeEvent>& events
)
{
    for (const auto& event : events)
    {
        // get_if sees if this event is a Trade,
        // if so, we get a pointer to that Trade and if not then trade == nullptr
        if (const auto* trade =
                std::get_if<Trade>(&event))
        {
            (void)on_trade(*trade);
        }
    }
}

// this function is not const as next_order_id_ changes
QuoteOrders
MarketMaker::make_orders(
    Price fair_value,
    Quantity quantity
)
{
    const Quote quote =
        make_quote(fair_value);

    const Order bid{
        next_order_id_++,
        quote.bid_price,
        quantity,
        Side::Buy
    };

    const Order ask{
        next_order_id_++,
        quote.ask_price,
        quantity,
        Side::Sell
    };

    return {
        bid,
        ask
    };
}

void
MarketMaker::register_if_accepted(
    const Order& order,
    const std::vector<ExchangeEvent>& events
)
{
    for (const auto& event : events)
    {
        if (const auto* accepted =
                std::get_if<OrderAccepted>(&event))
        {
            if (accepted->order_id != order.id)
            {
                continue;
            }

            const ActiveOrder active{
                order.id,
                order.quantity
            };

            if (order.side == Side::Buy)
            {
                active_bid_ = active;
            }
            else
            {
                active_ask_ = active;
            }

            return;
        }
    }
}

void
MarketMaker::submit_quotes(
    Exchange& exchange,
    Price fair_value,
    Quantity quantity
)
{
    const QuoteOrders orders =
        make_orders(fair_value, quantity);

    if (can_submit(Side::Buy, quantity))
    {
        auto bid_events =
            exchange.submit(orders.bid);

        register_if_accepted(
            orders.bid,
            bid_events
        );

        on_events(bid_events);
    }

    if (can_submit(Side::Sell, quantity))
    {
        auto ask_events =
            exchange.submit(orders.ask);

        register_if_accepted(
            orders.ask,
            ask_events
        );

        on_events(ask_events);
    }
}

bool
MarketMaker::was_cancelled(
    OrderId id,
    const std::vector<ExchangeEvent>& events
) const
{
    for (const auto& event : events)
    {
        if (const auto* cancelled =
                std::get_if<OrderCancelled>(&event))
        {
            if (cancelled->order_id == id)
            {
                return true;
            }
        }
    }

    return false;
}

bool
MarketMaker::cancel_active_quotes(
    Exchange& exchange
)
{
    bool success = true;

    if (active_bid_.has_value())
    {
        const OrderId id =
            active_bid_->id;

        const auto events =
            exchange.cancel(id);

        if (was_cancelled(id, events))
        {
            active_bid_.reset();
        }
        else
        {
            success = false;
        }
    }

    if (active_ask_.has_value())
    {
        const OrderId id =
            active_ask_->id;

        const auto events =
            exchange.cancel(id);

        if (was_cancelled(id, events))
        {
            active_ask_.reset();
        }
        else
        {
            success = false;
        }
    }

    return success;
}

bool
MarketMaker::refresh_quotes(
    Exchange& exchange,
    Price fair_value,
    Quantity quantity
)
{
    if (!cancel_active_quotes(exchange))
    {
        return false;
    }

    submit_quotes(
        exchange,
        fair_value,
        quantity
    );

    return true;
}

std::optional<Price>
MarketMaker::fair_value_from_market(
    const Exchange& exchange
) const
{
    const auto top =
        exchange.top_of_book();

    if (
        !top.best_bid.has_value() ||
        !top.best_ask.has_value()
    )
    {
        return std::nullopt;
    }

    return *top.best_bid +
           (*top.best_ask - *top.best_bid) / 2;
}

// using formula: 
// microprice =
// (ask_price × bid_qty + bid_price × ask_qty)
//------------------------------------------------
//              bid_qty + ask_qty
//
// we see the weighting is opposite side price x quantity, so a heavy big quantity
// pulls the estimate towards the ask
std::optional<Price>
MarketMaker::microprice_from_market(
    const Exchange& exchange
) const
{
    const auto top =
        exchange.top_of_book();

    if (
        !top.best_bid.has_value() ||
        !top.best_ask.has_value() ||
        !top.best_bid_quantity.has_value() ||
        !top.best_ask_quantity.has_value()
    )
    {
        return std::nullopt;
    }

    const long double bid_price =
        static_cast<long double>(
            *top.best_bid
        );

    const long double ask_price =
        static_cast<long double>(
            *top.best_ask
        );

    const long double bid_quantity =
        static_cast<long double>(
            *top.best_bid_quantity
        );

    const long double ask_quantity =
        static_cast<long double>(
            *top.best_ask_quantity
        );

    const long double total_quantity =
        bid_quantity + ask_quantity;

    if (total_quantity == 0)
    {
        return std::nullopt;
    }

    const long double microprice =
        (
            ask_price * bid_quantity +
            bid_price * ask_quantity
        ) / total_quantity;

    return static_cast<Price>(
        std::llround(microprice)
    );
}

bool
MarketMaker::refresh_quotes_from_market(
    Exchange& exchange,
    Quantity quantity
)
{
    if (!cancel_active_quotes(exchange))
    {
        return false;
    }

    auto fair_value =
        microprice_from_market(exchange);

    if (!fair_value.has_value())
    {
        fair_value =
            fair_value_from_market(exchange);
    }

    if (!fair_value.has_value())
    {
        return false;
    }

    submit_quotes(
        exchange,
        *fair_value,
        quantity
    );

    return true;
}

bool
MarketMaker::can_submit(
    Side side,
    Quantity quantity
) const
{
    const auto signed_quantity =
        static_cast<std::int64_t>(quantity);

    if (side == Side::Buy)
    {
        return signed_quantity <=
               max_inventory_ - inventory_;
    }

    return signed_quantity <=
           max_inventory_ + inventory_;
}

std::int64_t
MarketMaker::cash() const
{
    return cash_;
}

std::int64_t
MarketMaker::mark_to_market_pnl(
    Price mark_price
) const
{
    return cash_ +
           inventory_ * mark_price;
}


}