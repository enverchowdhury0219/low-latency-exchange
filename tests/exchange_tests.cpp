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
