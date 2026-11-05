#pragma once

#include <cstdint>

namespace exchange
{

// {0} gives every counter a safe initial value
struct ExchangeStats
{
    std::uint64_t orders_submitted{0};
    std::uint64_t orders_accepted{0};
    std::uint64_t orders_rejected{0};

    std::uint64_t trades_executed{0};
    std::uint64_t traded_quantity{0};

    std::uint64_t cancels_accepted{0};
    std::uint64_t cancels_rejected{0};

    std::uint64_t replaces_accepted{0};
    std::uint64_t replaces_rejected{0};
};

}