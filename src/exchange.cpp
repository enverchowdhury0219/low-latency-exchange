#include "exchange/exchange.hpp"

namespace exchange
{

std::vector<ExchangeEvent>
Exchange::submit(Order order)
{
    std::vector<ExchangeEvent> events;

    const auto result = book_.submit(order);

    if (!result.accepted)
    {
        // .push_back is a vector function to add new element to end of container
        // so adding the order being rejected to our exchange events vector
        events.push_back(
            OrderRejected{
                order.id,
                result.reject_reason
            }
        );

        return events;
    }

    // adding the suucessful add order event to our event vector
    events.push_back(
        OrderAccepted{
            order.id
        }
    );

    for (const auto& trade : result.trades)
    {
        // logging each trade that takes place as an event
        events.push_back(trade);
    }

    return events;
}

}