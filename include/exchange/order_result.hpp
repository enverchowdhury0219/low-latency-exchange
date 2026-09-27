#pragma once

#include<vector>

#include "exchange/trade.hpp"

namespace exchange
{

// some rejection reasons, so we display these instead of throwing exceptions
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