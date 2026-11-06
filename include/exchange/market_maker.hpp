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
    MarketMaker(
        Price half_spread
    );

    [[nodiscard]] Quote
    make_quote(Price fair_value) const;

private:
    Price half_spread_;
};

}