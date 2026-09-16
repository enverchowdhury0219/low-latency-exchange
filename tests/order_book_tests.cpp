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

// testing if our orderbook can be swept when a large order comes in
void test_multi_fill_sweep()
{
    exchange::OrderBook book;

    (void)book.submit({
        1,
        10130,
        50,
        exchange::Side::Sell
    });

    (void)book.submit({
        2,
        10130,
        75,
        exchange::Side::Sell
    });

    (void)book.submit({
        3,
        10135,
        100,
        exchange::Side::Sell
    });

    (void)book.submit({
        4,
        10145,
        200,
        exchange::Side::Sell
    });

        const auto trades = book.submit({
        20,
        10140,
        200,
        exchange::Side::Buy
    });

    expect(trades.size() == 3,
           "Sweep should produce three trades");

    expect(trades[0].price == 10130,
           "First trade price should be 10130");

    expect(trades[0].quantity == 50,
           "First trade quantity should be 50");

    expect(trades[1].price == 10130,
           "Second trade price should be 10130");

    expect(trades[1].quantity == 75,
           "Second trade quantity should be 75");

    expect(trades[2].price == 10135,
           "Third trade price should be 10135");

    expect(trades[2].quantity == 75,
           "Third trade quantity should be 75");

    const auto ask = book.best_ask();

    expect(ask.has_value(),
           "Best ask should still exist");

    expect(*ask == 10135,
           "Best ask should be 10135 after sweep");

           
}

//partial-fill test
void test_partial_fill()
{
    exchange::OrderBook book;

    (void)book.submit({
        1,
        10130,
        150,
        exchange::Side::Sell
    });

    const auto trades = book.submit({
        2,
        10140,
        100,
        exchange::Side::Buy
    });

    expect(trades.size() == 1,
           "Partial fill should produce one trade");

    expect(trades[0].quantity == 100,
           "Trade quantity should be 100");

    expect(trades[0].price == 10130,
           "Trade should execute at resting price");

    const auto ask = book.best_ask();

    expect(ask.has_value(),
           "Ask should remain after partial fill");

    expect(*ask == 10130,
           "Remaining seller should still be best ask");
}







// running both tests to ensure they pass at all times
int main()
{
    try
    {
        test_best_bid_and_ask();
        test_multi_fill_sweep();

        std::cout << "All order book tests passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "TEST FAILED: "
                  << error.what()
                  << '\n';

        return 1;
    }
}