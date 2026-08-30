#include <iostream>

#include "exchange/order_book.hpp"

// added 3 buy orders but 2 bid price levels as 2 orders have same price
int main()
{
    exchange::OrderBook sell_test_book;

    sell_test_book.add_order({
        100,
        10125,
        150,
        exchange::Side::Buy
    });

    sell_test_book.add_order({
        101,
        10120,
        200,
        exchange::Side::Buy
    });

    exchange::Order aggressive_sell{
        200,
        10110,
        100,
        exchange::Side::Sell
    };

    const auto sell_trade =
        sell_test_book.execute_one(aggressive_sell);

    if (sell_trade)
    {
        std::cout << "\nSell-side trade executed\n";
        std::cout << "Buyer: "
                << sell_trade->buy_order_id << '\n';

        std::cout << "Seller: "
                << sell_trade->sell_order_id << '\n';

        std::cout << "Price: "
                << sell_trade->price << '\n';

        std::cout << "Quantity: "
                << sell_trade->quantity << '\n';
    }

    std::cout << "Incoming sell remaining: "
            << aggressive_sell.quantity
            << '\n';

    std::cout << "Best bid after trade: "
            << *sell_test_book.best_bid()
            << '\n';
    return 0;
}

