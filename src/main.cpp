#include <iostream>

#include "exchange/exchange.hpp"
#include "exchange/market_maker.hpp"

int main()
{
    exchange::Exchange exchange;

    exchange::MarketMaker maker{
        5,      // half spread
        2,      // inventory skew
        100,    // inventory unit
        500     // max inventory
    };

    // --------------------------------------------------
    // 1. Seed an external market
    // --------------------------------------------------

    (void)exchange.submit({
        1,
        10090,
        300,
        exchange::Side::Buy
    });

    (void)exchange.submit({
        2,
        10110,
        100,
        exchange::Side::Sell
    });

    const auto initial_fair_value =
        maker.microprice_from_market(exchange);

    if (!initial_fair_value.has_value())
    {
        std::cerr
            << "Could not calculate initial fair value.\n";

        return 1;
    }

    std::cout
        << "Initial microprice: "
        << *initial_fair_value
        << '\n';

    // --------------------------------------------------
    // 2. Market maker posts its first quote
    // --------------------------------------------------

    if (!maker.refresh_quotes_from_market(
            exchange,
            100))
    {
        std::cerr
            << "Could not submit initial quotes.\n";

        return 1;
    }

    // External seller hits our bid.
    const auto sell_events =
        exchange.submit({
            3,
            10100,
            100,
            exchange::Side::Sell
        });

    maker.on_events(sell_events);

    std::cout
        << "After bid fill:\n"
        << "  Inventory: "
        << maker.inventory()
        << '\n'
        << "  Cash: "
        << maker.cash()
        << '\n';

    // --------------------------------------------------
    // 3. Requote while long
    // --------------------------------------------------

    if (!maker.refresh_quotes_from_market(
            exchange,
            100))
    {
        std::cerr
            << "Could not refresh quotes.\n";

        return 1;
    }

    // Because inventory is +100, the strategy's
    // quote is skewed downward.
    //
    // New ask should be 10108.
    //
    // External buyer takes that ask.

    const auto buy_events =
        exchange.submit({
            4,
            10108,
            100,
            exchange::Side::Buy
        });

    maker.on_events(buy_events);

    // --------------------------------------------------
    // 4. Final strategy results
    // --------------------------------------------------

    std::cout
        << "\nFinal strategy state:\n"
        << "  Inventory: "
        << maker.inventory()
        << '\n'
        << "  Cash: "
        << maker.cash()
        << '\n'
        << "  Realized PnL: "
        << maker.realized_pnl()
        << '\n'
        << "  Unrealized PnL: "
        << maker.unrealized_pnl(10105)
        << '\n'
        << "  Total PnL: "
        << maker.total_pnl(10105)
        << '\n';

    const auto& stats =
        exchange.stats();

    std::cout
        << "\nExchange statistics:\n"
        << "  Orders submitted: "
        << stats.orders_submitted
        << '\n'
        << "  Orders accepted: "
        << stats.orders_accepted
        << '\n'
        << "  Orders rejected: "
        << stats.orders_rejected
        << '\n'
        << "  Trades executed: "
        << stats.trades_executed
        << '\n'
        << "  Traded quantity: "
        << stats.traded_quantity
        << '\n'
        << "  Cancels accepted: "
        << stats.cancels_accepted
        << '\n';

    return 0;
}

