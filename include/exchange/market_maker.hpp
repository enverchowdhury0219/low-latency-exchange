#pragma once

#include "exchange/order.hpp"

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
    MarketMaker(
        Price half_spread,
        Price inventory_skew = 0
    );

    [[nodiscard]] Quote
    make_quote(Price fair_value) const;

    void set_inventory(std::int64_t inventory);

    [[nodiscard]] std::int64_t
    inventory() const;

private:
    Price half_spread_;
    Price inventory_skew_;

    std::int64_t inventory_{0};
};

}