#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "exchange/market_maker.hpp"
#include "exchange/exchange.hpp"


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

// test order generation
void test_market_maker_creates_orders()
{
    exchange::MarketMaker maker{5, 2};

    const auto orders =
        maker.make_orders(
            10100,
            100
        );

    expect(
        orders.bid.price == 10095,
        "Bid order should use quoted bid price"
    );

    expect(
        orders.ask.price == 10105,
        "Ask order should use quoted ask price"
    );

    expect(
        orders.bid.quantity == 100,
        "Bid should use requested quantity"
    );

    expect(
        orders.ask.quantity == 100,
        "Ask should use requested quantity"
    );

    expect(
        orders.bid.side == exchange::Side::Buy,
        "Bid should be a buy order"
    );

    expect(
        orders.ask.side == exchange::Side::Sell,
        "Ask should be a sell order"
    );

    expect(
        orders.bid.id != orders.ask.id,
        "Bid and ask must have unique IDs"
    );
}

// test that IDs keep advancing
void test_market_maker_order_ids_advance()
{
    exchange::MarketMaker maker{5, 2};

    const auto first =
        maker.make_orders(10100, 100);

    const auto second =
        maker.make_orders(10100, 100);

    expect(
        second.bid.id > first.ask.id,
        "New quotes should receive newer order IDs"
    );
}

void test_submit_quotes_processes_real_fills()
{
    exchange::Exchange exchange;

    // Existing seller provides 40 shares of liquidity.
    (void)exchange.submit({
        1,
        10095,
        40,
        exchange::Side::Sell
    });

    exchange::MarketMaker maker{5, 2};

    // MM quote:
    // bid = 10095
    // ask = 10105
    maker.submit_quotes(
        exchange,
        10100,
        100
    );

    expect(
        maker.inventory() == 40,
        "Immediate bid fill should create long inventory"
    );

    // Another seller hits the remaining 60-share bid.
    const auto events =
        exchange.submit({
            2,
            10095,
            60,
            exchange::Side::Sell
        });

    maker.on_events(events);

    expect(
        maker.inventory() == 100,
        "Second fill should bring inventory to 100"
    );
}

// test that old quotes disappear
void test_refresh_cancels_old_quotes()
{
    exchange::Exchange exchange;
    exchange::MarketMaker maker{5, 2};

    maker.submit_quotes(
        exchange,
        10100,
        100
    );

    // Original:
    // bid = 10095
    // ask = 10105

    const bool refreshed =
        maker.refresh_quotes(
            exchange,
            10120,
            100
        );

    expect(
        refreshed,
        "Quote refresh should succeed"
    );

    // New:
    // bid = 10115
    // ask = 10125
    //
    // A BUY at 10105 should NOT trade.
    // If the old 10105 ask still existed,
    // it would execute.

    const auto events =
        exchange.submit({
            1,
            10105,
            100,
            exchange::Side::Buy
        });

    bool trade_found = false;

    for (const auto& event : events)
    {
        if (std::get_if<exchange::Trade>(&event))
        {
            trade_found = true;
        }
    }

    expect(
        !trade_found,
        "Old ask should be removed after requoting"
    );

    expect(
        exchange.stats().cancels_accepted == 2,
        "Refreshing should cancel old bid and ask"
    );
}

void test_market_midpoint_fair_value()
{
    exchange::Exchange exchange;

    (void)exchange.submit({
        1,
        10090,
        100,
        exchange::Side::Buy
    });

    (void)exchange.submit({
        2,
        10110,
        100,
        exchange::Side::Sell
    });

    exchange::MarketMaker maker{5, 2};

    const auto fair_value =
        maker.fair_value_from_market(exchange);

    expect(
        fair_value.has_value(),
        "Two-sided market should produce fair value"
    );

    expect(
        *fair_value == 10100,
        "Fair value should equal market midpoint"
    );
}

void test_fair_value_requires_two_sided_market()
{
    exchange::Exchange exchange;

    (void)exchange.submit({
        1,
        10090,
        100,
        exchange::Side::Buy
    });

    exchange::MarketMaker maker{5, 2};

    const auto fair_value =
        maker.fair_value_from_market(exchange);

    expect(
        !fair_value.has_value(),
        "One-sided market should not produce midpoint"
    );
}

void test_microprice_moves_toward_heavier_bid()
{
    exchange::Exchange exchange;

    (void)exchange.submit({
        1,
        10090,
        300,
        exchange::Side::Buy
    });

    (void)exchange.submit({
        2,
        10110,
        100,
        exchange::Side::Sell
    });

    exchange::MarketMaker maker{5, 2};

    const auto microprice =
        maker.microprice_from_market(exchange);

    expect(
        microprice.has_value(),
        "Two-sided market should produce microprice"
    );

    expect(
        *microprice == 10105,
        "Heavy bid should shift microprice toward ask"
    );
}

void test_balanced_microprice_equals_midpoint()
{
    exchange::Exchange exchange;

    (void)exchange.submit({
        1,
        10090,
        100,
        exchange::Side::Buy
    });

    (void)exchange.submit({
        2,
        10110,
        100,
        exchange::Side::Sell
    });

    exchange::MarketMaker maker{5, 2};

    const auto microprice =
        maker.microprice_from_market(exchange);

    expect(
        microprice.has_value(),
        "Balanced market should produce microprice"
    );

    expect(
        *microprice == 10100,
        "Balanced quantities should produce midpoint"
    );
}

void test_refresh_quotes_uses_market_fair_value()
{
    exchange::Exchange exchange;

    // External market:
    // bid = 10090 × 300
    // ask = 10110 × 100
    //
    // microprice = 10105

    (void)exchange.submit({
        1,
        10090,
        300,
        exchange::Side::Buy
    });

    (void)exchange.submit({
        2,
        10110,
        100,
        exchange::Side::Sell
    });

    exchange::MarketMaker maker{5, 2};

    const bool refreshed =
        maker.refresh_quotes_from_market(
            exchange,
            100
        );

    expect(
        refreshed,
        "Market-driven refresh should succeed"
    );

    const auto top =
        exchange.top_of_book();

    expect(
        top.best_bid.has_value(),
        "Book should have a best bid"
    );

    expect(
        top.best_ask.has_value(),
        "Book should have a best ask"
    );

    expect(
        *top.best_bid == 10100,
        "Market maker bid should be 5 ticks below microprice"
    );

    expect(
        *top.best_ask == 10110,
        "Best ask should remain 10110"
    );
}

// testing magnitude-based skew
void test_inventory_magnitude_changes_skew()
{
    exchange::MarketMaker maker{5, 2, 100};

    maker.set_inventory(200);

    const auto quote =
        maker.make_quote(10100);

    expect(
        quote.bid_price == 10091,
        "Long 200 inventory should lower bid by 4 ticks"
    );

    expect(
        quote.ask_price == 10101,
        "Long 200 inventory should lower ask by 4 ticks"
    );
}

void test_short_inventory_magnitude_changes_skew()
{
    exchange::MarketMaker maker{5, 2, 100};

    maker.set_inventory(-200);

    const auto quote =
        maker.make_quote(10100);

    expect(
        quote.bid_price == 10099,
        "Short 200 inventory should raise bid by 4 ticks"
    );

    expect(
        quote.ask_price == 10109,
        "Short 200 inventory should raise ask by 4 ticks"
    );
}

// tesing the long limit
void test_position_limit_blocks_excess_bid()
{
    exchange::Exchange exchange;

    exchange::MarketMaker maker{
        5,      // half spread
        2,      // inventory skew
        100,    // inventory unit
        200     // max inventory
    };

    maker.set_inventory(150);

    maker.submit_quotes(
        exchange,
        10100,
        100
    );

    const auto top =
        exchange.top_of_book();

    expect(
        !top.best_bid.has_value(),
        "Bid should be blocked when full fill would exceed position limit"
    );

    expect(
        top.best_ask.has_value(),
        "Ask should still be allowed when long"
    );
}

void test_position_limit_blocks_excess_ask()
{
    exchange::Exchange exchange;

    exchange::MarketMaker maker{
        5,
        2,
        100,
        200
    };

    maker.set_inventory(-150);

    maker.submit_quotes(
        exchange,
        10100,
        100
    );

    const auto top =
        exchange.top_of_book();

    expect(
        top.best_bid.has_value(),
        "Bid should still be allowed when short"
    );

    expect(
        !top.best_ask.has_value(),
        "Ask should be blocked when full fill would exceed short limit"
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
        test_full_fill_removes_active_order();
        test_partial_fill_keeps_order_active();
        test_market_maker_creates_orders();
        test_market_maker_order_ids_advance();
        test_submit_quotes_processes_real_fills();
        test_refresh_cancels_old_quotes();
        test_market_midpoint_fair_value();
        test_fair_value_requires_two_sided_market();
        test_microprice_moves_toward_heavier_bid();
        test_balanced_microprice_equals_midpoint();
        test_refresh_quotes_uses_market_fair_value();
        test_inventory_magnitude_changes_skew();
        test_short_inventory_magnitude_changes_skew();
        test_position_limit_blocks_excess_bid();
        test_position_limit_blocks_excess_ask();

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