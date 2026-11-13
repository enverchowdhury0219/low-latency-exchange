#pragma once

#include <optional>
#include <vector>
#include "exchange/order.hpp"
#include "exchange/trade.hpp"
#include "exchange/event.hpp"


namespace exchange
{

struct Quote
{
    Price bid_price;
    Price ask_price;
};

class MarketMaker
{
public:
    // inventory is net position of assets that market maker holds on books currently.
    // inventory skew is the intentional shifting of bid and ask quotes away from the mid-price
    // to incentivize trades that reduce a market's risky new position (q) back towards 0.
    MarketMaker(
        Price half_spread,
        Price inventory_skew = 0
    );

    [[nodiscard]] Quote
    make_quote(Price fair_value) const;

    // we used a signed value because:
    // inventory > 0 -> long
    // inventory = 0 -> flat
    // inventory < 0 -> short
    void set_inventory(std::int64_t inventory);

    // as our buy/sell can increase/decrease inventory
    void on_fill(
        Side side,
        Quantity quantity
    );

    [[nodiscard]] std::int64_t
    inventory() const;

    void set_active_orders(
        OrderId bid_order_id,
        OrderId ask_order_id
    );

    [[nodiscard]] bool
    on_trade(const Trade& trade);

    void on_events(
    const std::vector<ExchangeEvent>& events
);

private:
    Price half_spread_;
    Price inventory_skew_;

    std::int64_t inventory_{0};

    std::optional<OrderId> bid_order_id_;
    std::optional<OrderId> ask_order_id_;
};

}