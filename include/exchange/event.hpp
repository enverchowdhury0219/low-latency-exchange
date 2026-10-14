#pragma once

#include <variant>

#include "exchange/order.hpp"
#include "exchange/order_result.hpp"
#include "exchange/trade.hpp"

namespace exchange
{

//creating outlines for our different exchange events
struct OrderAccepted
{
    OrderId order_id;
};

struct OrderRejected 
{
    OrderId order_id;
    RejectReason reason;
};
  
// we use variant here to say that this object contains exactly one valid event type
using ExchangeEvent = 
    std::variant<
    OrderAccepted,
    OrderRejected,
    OrderCancelled,
    CancelRejected,
    OrderReplaced,
    ReplaceRejected,
    Trade
    >;

    struct OrderCancelled
    {
        OrderId order_id;
    };

    struct CancelRejected
    {
        OrderId order_id;
    };

    struct OrderReplaced
    {
        OrderId order_id;
    };

    struct ReplaceRejected
    {
        OrderId order_id;
        RejectReason reason;
    };


}