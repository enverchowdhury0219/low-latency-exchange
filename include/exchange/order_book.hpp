#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <map>
#include <optional>

#include "exchange/order.hpp"

namespace exchange
{

// building our orderbook logic, what actually stores these data points
class OrderBook
{
public:
    void add_order(const Order& order);
    [[nodiscard]] std::optional<Price> best_bid() const;
    [[nodiscard]] std::optional<Price> best_ask() const;

    [[nodiscard]] std::size_t bid_level_count() const;
    [[nodiscard]] std::size_t ask_level_count() const;
private:
    using OrdersAtPrice = std::deque<Order>;

    // third parameter here is the comparison function used to sort keys
    using BidLevels =
        std::map<Price, OrdersAtPrice, std::greater<Price>>; // we want the highest bid 

    using AskLevels =
        std::map<Price, OrdersAtPrice, std::less<Price>>; // we want the lowest ask

    BidLevels bids_;
    AskLevels asks_;
};

}
