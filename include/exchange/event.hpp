#pragma once

#include <variant>
#include <optional>

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

// we use the optional structure here as one side may not exist i.e.
// we could have a best bid but no best ask (called top of book)
struct TopOfBookUpdate
{
    std::optional<Price> best_price;
    std::optional<Price> best_ask;
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
    TopOfBookUpdate,
    Trade
    >;


}