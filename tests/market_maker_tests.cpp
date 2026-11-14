#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

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
        100,
        1002,
        100
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
        100,
        1002,
        100
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

void test_unrelated_trade_does_not_change_inventory()
{
    exchange::MarketMaker maker{5, 2};

    maker.set_active_orders(
        1001,
        100,
        1002,
        100
    );

    const exchange::Trade trade{
        3000,
        4000,
        10100,
        100
    };

    const bool involved =
        maker.on_trade(trade);

    expect(
        !involved,
        "Unrelated trade should be ignored"
    );

    expect(
        maker.inventory() == 0,
        "Unrelated trade should not change inventory"
    );
}

// testing an actual event vector and if the market maker stores the trade
void test_event_stream_updates_inventory()
{
    exchange::MarketMaker maker{5, 2};

    maker.set_active_orders(
        1001,
        100,
        1002,
        100
    );

    std::vector<exchange::ExchangeEvent> events;

    events.push_back(
        exchange::OrderAccepted{2000}
    );

    events.push_back(
        exchange::Trade{
            1001,
            2000,
            10095,
            60
        }
    );

    events.push_back(
        exchange::TopOfBookUpdate{
            10095,
            40,
            10105,
            100
        }
    );

    maker.on_events(events);

    expect(
        maker.inventory() == 60,
        "Trade event should update inventory"
    );
}

void test_full_fill_removes_active_order()
{
    exchange::MarketMaker maker{5, 2};

    maker.set_active_orders(
        1001,
        100,
        1002,
        100
    );

    const exchange::Trade first_trade{
        1001,
        2000,
        10095,
        100
    };

    expect(
        maker.on_trade(first_trade),
        "Full bid fill should be recognized"
    );

    expect(
        maker.inventory() == 100,
        "Inventory should increase after fill"
    );

    const exchange::Trade duplicate_trade{
        1001,
        3000,
        10095,
        50
    };

    expect(
        !maker.on_trade(duplicate_trade),
        "Filled bid should no longer be active"
    );

    expect(
        maker.inventory() == 100,
        "Inactive order must not change inventory"
    );
}

void test_partial_fill_keeps_order_active()
{
    exchange::MarketMaker maker{5, 2};

    maker.set_active_orders(
        1001,
        100,
        1002,
        100
    );

    const exchange::Trade first_trade{
        1001,
        2000,
        10095,
        40
    };

    const exchange::Trade second_trade{
        1001,
        2001,
        10095,
        60
    };

    expect(
        maker.on_trade(first_trade),
        "Partial fill should be recognized"
    );

    expect(
        maker.on_trade(second_trade),
        "Remaining quantity should still be active"
    );

    expect(
        maker.inventory() == 100,
        "Both fills should total 100 inventory"
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
        test_trade_on_bid_increases_inventory();
        test_trade_on_ask_decreases_inventory();
        test_unrelated_trade_does_not_change_inventory();
        test_event_stream_updates_inventory();

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