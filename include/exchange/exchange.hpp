#pragma once

#include <vector>

#include "exchange/event.hpp"
#include "exchange/order_book.hpp"

namespace exchange
{

class Exchange
{
public:
    [[nodiscard]] std::vector<ExchangeEvent>
    submit(Order order);

    // added our cancel and replace event outline
    [[nodiscard]] std::vector<ExchangeEvent>
    cancel(OrderId id);

    [[nodiscard]] std::vector<ExchangeEvent>
    replace(
        OrderId id,
        Price new_price,
        Quantity new_quantity
    );

private:
    OrderBook book_;
};




}