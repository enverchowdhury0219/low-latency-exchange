#pragma once

#include <cstdint>

namespace exchange
{

using OrderId = std::uint64_t;
using Price = std::int64_t;
using Quantity = std::uint32_t;

enum class Side
{
    Buy,
    Sell
};

// creating our order structure for the order book
struct Order
{
    OrderId id; // 8 bytes
    Price price; // 8 bytes
    Quantity quantity; // 4 bytes
    Side side; // usually 4 bytes (typical for scoped enums)
};

}