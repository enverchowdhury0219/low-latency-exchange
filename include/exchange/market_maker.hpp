#pragma once

#include "exchange/order.hpp"

namespace exchange
{

struct Quite
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
}
}