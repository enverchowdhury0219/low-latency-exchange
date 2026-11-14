#include "exchange/market_maker.hpp"

namespace exchange
{

// half_spread is the param passed into constructor
// half_spread_ is member var stored in obj (the extra _ usually names private member vars)
MarketMaker::MarketMaker(
    Price half_spread,
    Price inventory_skew
)
    // : here starts memver initializer list
    // so when making this MarketMaker obj, init its half_spread_ using param half_spread
    // this direct init helps to avoid later work
    : half_spread_(half_spread),
        inventory_skew_(inventory_skew)

// this is our constructor body (req even if empty), empty bc we did what we needed to in initializer list
{
}   

// read-only member function
Quote
MarketMaker::make_quote(Price fair_value) const
{
    Price adjusted_fair_value = fair_value;

    // if long, we own too much, so shift down, so lower bid and ask
    if (inventory_ > 0)
    {
        adjusted_fair_value -= inventory_skew_;
    }
    // if short, we do the opposite
    else if (inventory_ < 0)
    {
        adjusted_fair_value += inventory_skew_;
    }

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
    Quantity quantity
)
{
    // we convert Quantity because it is unsigned, but inventory can be negative,
    // so we convert quantities before subtracting them
    const auto signed_quantity =
        static_cast<std::int64_t>(quantity);

    if (side == Side::Buy)
    {
        inventory_ += signed_quantity;
    }
    else
    {
        inventory_ -= signed_quantity;
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

}