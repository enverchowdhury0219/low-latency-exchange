#include <iostream>

#include "exchange/order_book.hpp"

// added 3 buy orders but 2 bid price levels as 2 orders have same price
int main()
{
    exchange::OrderBook remainder_book;

    // since these are setup fill orders, we know no trade will occur so we choose to ignore submits value
    (void)remainder_book.submit({
        1,
        10130,
        50,
        exchange::Side::Sell
    });


    (void)remainder_book.submit({
        2,
        10135,
        75,
        exchange::Side::Sell
    });

    (void)remainder_book.submit({
        3,
        10145,
        100,
        exchange::Side::Sell
    });

    exchange::Order incoming_buy{
    20,
    10140,
    200,
    exchange::Side::Buy
    };

    const auto remainder_trades = remainder_book.submit(incoming_buy);

    std::cout << "\nRemainder test\n";

    for (const auto& trade : remainder_trades)
    {
        std::cout
            << "Trade: "
            << trade.quantity
            << " @ "
            << trade.price
            << '\n';
    }

    if (const auto bid = remainder_book.best_bid())
    {
        std::cout << "Best bid after submission: "
                << *bid
                << '\n';
    }

    if (const auto ask = remainder_book.best_ask())
    {
        std::cout << "Best ask after submission: "
                << *ask
                << '\n';
    }

}

