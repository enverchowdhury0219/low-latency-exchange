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
    OrderId ask_order_id
)
{
    bid_order_id_ = bid_order_id;
    ask_order_id_ = ask_order_id;
}

bool
MarketMaker::on_trade(
    const Trade& trade
)
{
    bool involved = false;

    if (
        bid_order_id_.has_value() &&
        trade.buy_order_id == *bid_order_id_
    )
    {
        on_fill(
            Side::Buy,
            trade.quantity
        );

        involved = true;
    }

    if (
        ask_order_id_.has_value() &&
        trade.sell_order_id == *ask_order_id_
    )
    {
        on_fill(
            Side::Sell,
            trade.quantity
        );

        involved = true;
    }

    return involved;
}

}