#include <iostream>
#include <stdexcept>

#include "exchange/order_book.hpp"

// added 3 buy orders but 2 bid price levels as 2 orders have same price
int main()
{
    exchange::OrderBook duplicate_test;

    (void)duplicate_test.submit({
        42,
        10120,
        100,
        exchange::Side::Buy
    });

    try
    {
        (void)duplicate_test.submit({
            42,
            10110,
            50,
            exchange::Side::Buy
        });
    }
    catch (const std::invalid_argument& error)
    {
        std::cout << "Rejected duplicate: "
                << error.what()
                << '\n';
    }

    


}

