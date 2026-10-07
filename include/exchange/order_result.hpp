#pragma once

#include<vector>

#include "exchange/trade.hpp"

namespace exchange
{

// rejection reasons, so we display these instead of throwing exceptions
enum class RejectReason
{
    None,
    InvalidOrderId,
    InvalidQuantity,
    DuplicateOrderId
};

struct SubmitResult
{
    bool accepted;
    RejectReason reject_reason;
    std::vector<Trade> trades;
};

// added a replacement result struct for better testabiltiy 
struct ReplaceResult
{
    bool replaced;
    RejectReason reject_reason;
    std::vector<Trade> trades;
};

}