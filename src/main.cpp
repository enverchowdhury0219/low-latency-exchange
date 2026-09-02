#include <iostream>
#include <stdexcept>

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

    try
    {
        (void)remainder_book.submit({
            0,
            10150,
            100,
            exchange::Side::Buy
        });
    }
    catch (const std::invalid_argument& error)
    {
        std::cout << "Rejected order: "
                << error.what()
                << '\n';
    }

    try
    {
        (void)remainder_book.submit({
            99,
            10150,
            0,
            exchange::Side::Buy
        });
    }
    catch (const std::invalid_argument& error)
    {
        std::cout << "Rejected order: "
                << error.what()
                << '\n';
    }



}

