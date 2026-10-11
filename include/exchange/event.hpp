#pragma once

#include <variant>

#include "exchange/order.hpp"
#include "exchange/order_result.hpp"
#include "exchange/trade.hpp"

namespace exchange
{

    struct OrderAccepted
    {
        OrderId order_id;
    };
}