// created first test file

#include <iostream>
#include <stdexcept>
#include <string>

#include "exchange/order_book.hpp"

// basic expect logic for our tests
void expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}


// testing if our best bid and ask function works 
void test_best_bid_and_ask()
{
    exchange::OrderBook book;

    (void)book.submit({
        1,
        10125,
        100,
        exchange::Side::Buy
    });

    (void)book.submit({
        2,
        10125,
        200,
        exchange::Side::Buy
    });

     (void)book.submit({
        3,
        10130,
        150,
        exchange::Side::Sell
    });

    (void)book.submit({
        4,
        10135,
        75,
        exchange::Side::Sell
    });

    const auto bid = book.best_bid();
    const auto ask = book.best_ask();

    expect(bid.has_value(), "Best bid should exist");
    expect(ask.has_value(), "Best ask should exist");

    expect(*bid == 10125, "Best bid should be 10125");
    expect(*ask == 10130, "Best ask should be 10130");
    
}

void test_multi_order_sweep()
{
    exchange::OrderBook book;
}