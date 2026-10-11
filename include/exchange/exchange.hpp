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

private:
    OrderBook book_;
};

}