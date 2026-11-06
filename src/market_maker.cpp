#include "exchange/market_maker.hpp"

namespace exchange
{

MarketMaker::MarketMaker(
    Price half_spread
)
    : half_spread_(half_spread)
{
}

Quote
MarketMaker::make_quote(Price fair_value) const
{
    return {
        fair_value - half_spread_,
        fair_value + half_spread_
    };
}

}