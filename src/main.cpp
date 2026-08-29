#include <iostream>

#include "exchange/order_book.hpp"

// added 3 buy orders but 2 bid price levels as 2 orders have same price
int main()
{
    exchange::OrderBook book;

    book.add_order({
        1,
        10125,
        100,
        exchange::Side::Buy
    });

    book.add_order({
        2,
        10120,
        200,
        exchange::Side::Buy
    });

    book.add_order({
        3,
        10130,
        150,
        exchange::Side::Sell
    });

    book.add_order({
        4,
        10135,
        75,
        exchange::Side::Sell
    });

    book.add_order({
        5,
        10125,
        50,
        exchange::Side::Buy
    });

    const auto best_bid = book.best_bid();
    const auto best_ask = book.best_ask();

    if (best_bid)
    {
        std::cout << "Best bid: " << *best_bid << '\n';
    }

    if (best_ask)
    {
        std::cout << "Best ask: " << *best_ask << '\n';
    }

    std::cout << "Bid levels: "
              << book.bid_level_count()
              << '\n';

    std::cout << "Ask levels: "
              << book.ask_level_count()
              << '\n';

    if (best_bid && best_ask)
    {
        std::cout << "Spread: "
                  << (*best_ask - *best_bid)
                  << " ticks\n";
    }

    exchange::Order non_crossing_buy{
    6,
    10128,
    100,
    exchange::Side::Buy
    };

    // crossing as this buy price is higher than the selling price
    exchange::Order crossing_buy{
    7,
    10130,
    100,
    exchange::Side::Buy
    };

    exchange::Order crossing_sell{
    8,
    10125,
    100,
    exchange::Side::Sell
    };

    std::cout << std::boolalpha;

    std::cout << "Buy @ 10128 crosses: "
        << book.would_cross(non_crossing_buy)
        << '\n';

    std::cout << "Buy @ 10130 crosses: "
        << book.would_cross(crossing_buy)
        << '\n';

    std::cout << "Sell @ 10125 crosses: "
        << book.would_cross(crossing_sell)
        << '\n';

    exchange::Order buy_below_ask{
    9,
    10128,
    100,
    exchange::Side::Buy
    };

    exchange::Order buy_through_ask{
    10,
    10140,
    100,
    exchange::Side::Buy
    };

    exchange::Order sell_through_bid{
    11,
    10120,
    100,
    exchange::Side::Sell
    };

    const auto price1 = book.execution_price(buy_below_ask);
    const auto price2 = book.execution_price(buy_through_ask);
    const auto price3 = book.execution_price(sell_through_bid);

    if (price1)
    {
        std::cout << "Buy @ 10128 executes at: "
                << *price1 << '\n';
    }
    else
    {
        std::cout << "Buy @ 10128 does not execute\n";
    }

    if (price2)
    {
        std::cout << "Buy @ 10140 executes at: "
                << *price2 << '\n';
    }

    if (price3)
    {
        std::cout << "Sell @ 10120 executes at: "
                << *price3 << '\n';
    }

    return 0;
}

