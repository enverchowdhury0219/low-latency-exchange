#include <iostream>
#include <stdexcept>

#include "exchange/order_book.hpp"

int main()
{
    exchange::OrderBook book;

     (void)book.submit({
        1,
        10130,
        100,
        exchange::Side::Sell
    });

    (void)book.submit({
        2,
        10135,
        100,
        exchange::Side::Sell
    });

     // our incoming trade of buying 150 for 10140
    const auto trades = book.submit({
        3,
        10140,
        150,
        exchange::Side::Buy
    });

    for (const auto& trade: trades)
    {
        std::cout
            << "Trade: "
            << trade.quantity
            << " @ "
            << trade.price
            << '\n';
    }

    if (const auto bid = book.best_bid())
    {
        std::cout << "Best bid: " << *bid << '\n';
    }

    if (const auto ask = book.best_ask())
    {
        std::cout << "Best ask: " << *ask << '\n';
    }

    return 0;
}


