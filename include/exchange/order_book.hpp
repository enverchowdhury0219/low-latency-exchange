#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <vector>

#include "exchange/order.hpp"
#include "exchange/trade.hpp"


namespace exchange
{

// building our orderbook logic, what actually stores these data points
class OrderBook
{
public:
    [[nodiscard]] std::vector<Trade>
    submit(Order incoming); // we accept the order by value here, so this function gets own copy to mutate

    [[nodiscard]] std::optional<Price> best_bid() const;
    [[nodiscard]] std::optional<Price> best_ask() const;

    [[nodiscard]] std::size_t bid_level_count() const;
    [[nodiscard]] std::size_t ask_level_count() const;

    [[nodiscard]] bool would_cross(const Order& order) const;

    [[nodiscard]] std::optional<Price>
    execution_price(const Order& order) const;


private:
    using OrdersAtPrice = std::deque<Order>;

    // third parameter here is the comparison function used to sort keys
    using BidLevels =
        std::map<Price, OrdersAtPrice, std::greater<Price>>; // we want the highest bid 

    using AskLevels =
        std::map<Price, OrdersAtPrice, std::less<Price>>; // we want the lowest ask

    void add_order(const Order& order);
   
    // we dont use const here as we plan on changing its remaining quantity
    [[nodiscard]] std::optional<Trade>
    execute_one(Order& incoming);

    [[nodiscard]] std::vector<Trade>
    execute(Order& order);
        
    BidLevels bids_;
    AskLevels asks_;
};

}
