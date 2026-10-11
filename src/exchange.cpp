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
        events.push_back(
            OrderRejected{
                order.id,
                result.reject_reason
            }
        );

        return events;
    }

    events.push_back(
        OrderAccepted{
            order.id
        }
    );

    for (const auto& trade : result.trades)
    {
        events.push_back(trade);
    }

    return events;
}

}