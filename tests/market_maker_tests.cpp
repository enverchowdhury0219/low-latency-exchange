#include <iostream>
#include <stdexcept>
#include <string>

#include "exchange/market_maker.hpp"

void expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void test_basic_quote()
{
    // defining our half spread as 5
    exchange::MarketMaker maker{5};

    const auto quote =
        maker.make_quote(10100);

    expect(quote.bid_price == 10095,
           "Bid should be 5 ticks below fair value");

    expect(quote.ask_price == 10105,
           "Ask should be 5 ticks above fair value");
}

void test_long_inventory_skews_quote_down()
{
    exchange::MarketMaker maker{5, 2};

    maker.set_inventory(100);

    const auto quote =
        maker.make_quote(10100);

    expect(
        quote.bid_price == 10093,
        "Long inventory should lower bid"
    );

    expect(
        quote.ask_price == 10103,
        "Long inventory should lower ask"
    );
}

void test_short_inventory_skews_quote_up()
{
    exchange::MarketMaker maker{5, 2};

    maker.set_inventory(-100);

    const auto quote =
        maker.make_quote(10100);

    expect(
        quote.bid_price == 10097,
        "Short inventory should raise bid"
    );

    expect(
        quote.ask_price == 10107,
        "Short inventory should raise ask"
    );
}

// testing that our fill works
void test_buy_fill_increases_inventory()
{
    exchange::MarketMaker maker{5, 2};

    maker.on_fill(
        exchange::Side::Buy,
        100
    );

    expect(
        maker.inventory() == 100,
        "Buy fill should increase inventory"
    );
}

void test_sell_fill_decreases_inventory()
{
    exchange::MarketMaker maker{5, 2};

    maker.on_fill(
        exchange::Side::Sell,
        75
    );

    expect(
        maker.inventory() == -75,
        "Sell fill should decrease inventory"
    );
}

// testing whether our future quotes are affected properly
void test_fills_affect_future_quotes()
{
    exchange::MarketMaker maker{5, 2};

    maker.on_fill(
        exchange::Side::Buy,
        100
    );

    const auto quote =
        maker.make_quote(10100);

    expect(
        quote.bid_price == 10093,
        "Long inventory should lower future bid"
    );

    expect(
        quote.ask_price == 10103,
        "Long inventory should lower future ask"
    );
}

void test_trade_on_bid_increases_inventory()
{
    exchange::MarketMaker maker{5, 2};

    maker.set_active_orders(
        1001,
        1002
    );

    const exchange::Trade trade{
        1001,
        2000,
        10095,
        50
    };

    const bool involved =
        maker.on_trade(trade);

    expect(
        involved,
        "Market maker should recognize its bid trade"
    );

    expect(
        maker.inventory() == 50,
        "Bid fill should increase inventory"
    );
}

void test_trade_on_ask_decreases_inventory()
{
    exchange::MarketMaker maker{5, 2};

    maker.set_active_orders(
        1001,
        1002
    );

    const exchange::Trade trade{
        2000,
        1002,
        10105,
        40
    };

    const bool involved =
        maker.on_trade(trade);

    expect(
        involved,
        "Market maker should recognize its ask trade"
    );

    expect(
        maker.inventory() == -40,
        "Ask fill should decrease inventory"
    );
}

int main()
{
    try
    {
        test_basic_quote();
        test_long_inventory_skews_quote_down();
        test_short_inventory_skews_quote_up();
        test_buy_fill_increases_inventory();
        test_sell_fill_decreases_inventory();
        test_fills_affect_future_quotes();

        std::cout
            << "All market maker tests passed.\n";

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr
            << "TEST FAILED: "
            << error.what()
            << '\n';

        return 1;
    }
}