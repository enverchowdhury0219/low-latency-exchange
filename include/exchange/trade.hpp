#pragma once

#include "exchange/order.hpp"

namespace exchange
{

struct Trade
{
    OrderId buy_order_id;
    OrderId sell_order_id;
    Price price;
    Quantity quantity;
};

}