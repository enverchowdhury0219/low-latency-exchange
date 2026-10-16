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

    events.push_back(top_of_book());

    return events;
}

std::vector<ExchangeEvent>
Exchange::cancel(OrderId id)
{
    std::vector<ExchangeEvent> events;

    if (book_.cancel(id))
    {
        events.push_back(
            OrderCancelled{id}
        );

        events.push_back(top_of_book());
    }
    else
    {
        events.push_back(
            CancelRejected{id}
        );
    }

    return events;
}

std::vector<ExchangeEvent>
Exchange::replace(
    OrderId id,
    Price new_price,
    Quantity new_quantity
)
{
    std::vector<ExchangeEvent> events;

    const auto result =
        book_.replace(id, new_price, new_quantity);

    if (!result.replaced)
    {
        events.push_back(
            ReplaceRejected{
                id,
                result.reject_reason
            }
        );

        return events;
    }

    events.push_back(
        OrderReplaced{id}
    );

    for (const auto& trade : result.trades)
    {
        events.push_back(trade);
    }
   
    // we only add the successful version of this event to our events
    events.push_back(top_of_book());

    return events;
}

TopOfBookUpdate
Exchange::top_of_book() const{
    return {
        book_.best_bid(),
        book_.best_ask()
    };
}

}