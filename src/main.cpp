#include <iostream>
#include <stdexcept>

#include "exchange/order_book.hpp"

// added 3 buy orders but 2 bid price levels as 2 orders have same price
int main()
{
    exchange::OrderBook reuse_test;

    (void)reuse_test.submit({
        50,
        10130,
        100,
        exchange::Side::Sell
    });

    const auto fill_trades = reuse_test.submit({
        51,
        10130,
        100,
        exchange::Side::Buy
    });

    try
    {
        (void)reuse_test.submit({
            50,
            10140,
            100,
            exchange::Side::Sell
        });

        std::cout << "Order ID 50 successfully reused\n";
    
    }
    catch (const std::invalid_argument& error)
    {
        std::cout << error.what() << '\n';
    }
    




}

