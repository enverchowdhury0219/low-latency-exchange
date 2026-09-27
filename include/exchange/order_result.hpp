#pragma once

#include<vector>

#include "exchange/trade.hpp"

namespace exchange
{


enum class RejectReason
{
None,
InvalidOrder,
InvalidQuantity,
DuplicateOrderId
};

struct SubmitResult
{
    bool accepted;
    RejectReason reject_reason;
    std::vector<Trade> trades;
};

}