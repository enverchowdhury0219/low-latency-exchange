#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>

#include "exchange/exchange.hpp"

void expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void test_accepted_order_event()
{
    exchange::Exchange exchange;

    const auto events = exchange.submit({
        1,
        10130,
        100,
        exchange::Side::Sell
    });

    expect(events.size() == 1,
           "Resting order should produce one event");

    const auto* accepted =
        std::get_if<exchange::OrderAccepted>(&events[0]);

    expect(accepted != nullptr,
           "Event should be OrderAccepted");

    expect(accepted->order_id == 1,
           "Accepted event should contain order ID 1");
}

void test_trade_event()
{
    exchange::Exchange exchange;

    (void)exchange.submit({
        1,
        10130,
        100,
        exchange::Side::Sell
    });

    const auto events = exchange.submit({
        2,
        10140,
        100,
        exchange::Side::Buy
    });

    expect(events.size() == 2,
           "Crossing order should produce acceptance and trade");

    const auto* accepted =
        std::get_if<exchange::OrderAccepted>(&events[0]);

    expect(accepted != nullptr,
           "First event should be OrderAccepted");

    const auto* trade =
        std::get_if<exchange::Trade>(&events[1]);

    expect(trade != nullptr,
           "Second event should be Trade");

    expect(trade->buy_order_id == 2,
           "Incoming order should be buyer");

    expect(trade->sell_order_id == 1,
           "Resting order should be seller");

    expect(trade->price == 10130,
           "Trade should execute at resting price");

    expect(trade->quantity == 100,
           "Trade quantity should be 100");
}

void test_rejected_order_event()
{
    exchange::Exchange exchange;

    const auto events = exchange.submit({
        0,
        10130,
        100,
        exchange::Side::Buy
    });

    expect(events.size() == 1,
           "Rejected order should produce one event");

    const auto* rejected =
        std::get_if<exchange::OrderRejected>(&events[0]);

    expect(rejected != nullptr,
           "Event should be OrderRejected");

    expect(
        rejected->reason ==
            exchange::RejectReason::InvalidOrderId,
        "Reject reason should be InvalidOrderId"
    );
}

// testing when an order is cancelled in the exchange
void test_cancel_event()
{
    exchange::Exchange exchange;

    (void)exchange.submit({
        10,
        10120,
        100,
        exchange::Side::Buy
    });

    const auto events = exchange.cancel(10);

    expect(events.size() == 1,
           "Cancel should produce one event");

    const auto* cancelled =
        std::get_if<exchange::OrderCancelled>(&events[0]);

    expect(cancelled != nullptr,
           "Event should be OrderCancelled");

    expect(cancelled->order_id == 10,
           "Cancelled event should contain order ID 10");
}

void test_cancel_rejected_event()
{
    exchange::Exchange exchange;

    const auto events = exchange.cancel(999);

    expect(events.size() == 1,
           "Rejected cancel should produce one event");

    const auto* rejected =
        std::get_if<exchange::CancelRejected>(&events[0]);

    expect(rejected != nullptr,
           "Event should be CancelRejected");

    expect(rejected->order_id == 999,
           "Rejected cancel should contain requested ID");
}

void test_replace_trade_events()
{
    exchange::Exchange exchange;

    (void)exchange.submit({
        1,
        10130,
        100,
        exchange::Side::Sell
    });

    (void)exchange.submit({
        10,
        10120,
        100,
        exchange::Side::Buy
    });

    const auto events =
        exchange.replace(10, 10135, 100);

    expect(events.size() == 2,
           "Crossing replace should produce replace and trade events");

    const auto* replaced =
        std::get_if<exchange::OrderReplaced>(&events[0]);

    expect(replaced != nullptr,
           "First event should be OrderReplaced");

    expect(replaced->order_id == 10,
           "Replace event should contain order ID 10");

    const auto* trade =
        std::get_if<exchange::Trade>(&events[1]);

    expect(trade != nullptr,
           "Second event should be Trade");

    expect(trade->buy_order_id == 10,
           "Replacement should be buyer");

    expect(trade->sell_order_id == 1,
           "Resting seller should be matched");

    expect(trade->price == 10130,
           "Trade should execute at resting price");

    expect(trade->quantity == 100,
           "Trade quantity should be 100");
}

int main()
{
    try
    {
        test_accepted_order_event();
        test_trade_event();
        test_rejected_order_event();
        test_cancel_event();
        test_cancel_rejected_event();
        test_replace_trade_events();

        std::cout << "All exchange tests passed.\n";
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