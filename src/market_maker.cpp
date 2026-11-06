#include "exchange/market_maker.hpp"

namespace exchange
{

// half_spread is the param passed into constructor
// half_spread_ is member var stored in obj (the extra _ usually names private member vars)
MarketMaker::MarketMaker(
    Price half_spread
)
    // : here starts memver initializer list
    // so when making this MarketMaker obj, init its half_spread_ using param half_spread
    // this direct init helps to avoid later work
    : half_spread_(half_spread)

// this is our constructor body (req even if empty), empty bc we did what we needed to in initializer list
{
}

// read-only member function
Quote
MarketMaker::make_quote(Price fair_value) const
{
    // our Quote object
    return {
        fair_value - half_spread_,
        fair_value + half_spread_
    };
}

}