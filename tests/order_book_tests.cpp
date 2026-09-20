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

// order cancellation test
void test_cancel_order()
{
    exchange::OrderBook book;

    (void)book.submit({
        10,
        10125,
        100,
        exchange::Side::Buy
    });

    (void)book.submit({
        11,
        10120,
        50,
        exchange::Side::Buy
    });

    expect(book.cancel(10),
           "Existing order should cancel successfully");

    const auto bid = book.best_bid();

    expect(bid.has_value(),
           "Best bid should still exist");

    expect(*bid == 10120,
           "Best bid should move to 10120 after cancel");

    expect(!book.cancel(999),
           "Missing order should not cancel");
}

// duplicate id rejection
void test_duplicate_order_id_rejected()
{
    exchange::OrderBook book;

    (void)book.submit({
        42,
        10120,
        100,
        exchange::Side::Buy
    });

    bool rejected = false;

    try
    {
        (void)book.submit({
            42,
            10110,
            50,
            exchange::Side::Buy
        });
    }
    catch (const std::invalid_argument&)
    {
        rejected = true;
    }

    expect(rejected,
           "Duplicate live order ID should be rejected");

}

// testing the price-time priority of the order book
void test_price_time_priority()
{
    exchange::OrderBook book;

    (void)book.submit({
        100,
        10130,
        50,
        exchange::Side::Sell
    });

    (void)book.submit({
        101,
        10130,
        50,
        exchange::Side::Sell
    });

    const auto trades = book.submit({
        200,
        10130,
        75,
        exchange::Side::Buy
    });

    expect(trades.size() == 2,
           "Incoming order should generate two trades");

    expect(trades[0].sell_order_id == 100,
           "Earlier order should execute first");

    expect(trades[0].quantity == 50,
           "First seller should fully fill");

    expect(trades[1].sell_order_id == 101,
           "Later order should execute second");

    expect(trades[1].quantity == 25,
           "Second seller should partially fill");

}

// testing concept that empty book cant have a bid/ask and nothing to cancel
void test_empty_book()
{
    exchange::OrderBook book;
    
    expect(!book.best_bid().has_value(),
           "Empty book should have no best bid");

    expect(!book.best_ask().has_value(),
           "Empty book should have no best ask");

    expect(!book.cancel(999),
           "Cancelling from empty book should fail");


}

// testing whether fully filled price level disappears
void test_fill_level_removed()
{
    exchange::OrderBook book;

    (void)book.submit({
        1,
        10130,
        100,
        exchange::Side::Sell
    });

    const auto trades = book.submit({
        2,
        10130,
        100,
        exchange::Side::Buy
    });

    expect(trades.size() == 1,
           "Full fill should produce one trade");

    expect(!book.best_ask().has_value(),
           "Filled ask level should disappear");

}

// running tests to ensure they pass at all times
int main()
{
    try
    {
        test_best_bid_and_ask();
        test_multi_fill_sweep();
        test_partial_fill();
        test_cancel_order();
        test_duplicate_order_id_rejected();
        test_price_time_priority();
        test_empty_book();

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