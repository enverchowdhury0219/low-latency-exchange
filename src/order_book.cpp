#include "exchange/order_book.hpp"
#include <algorithm> // for std::min
#include <stdexcept> // for throwing exceptions

namespace exchange
{

void OrderBook::add_order(const Order& order)
{
    if (order.side == Side::Buy)
    {
        bids_[order.price].push_back(order); // adding a new order to back of that price's queue
    }
    else
    {
        asks_[order.price].push_back(order);
    }
}

// we use optional here to prevent using 0 as a null value
std::optional<Price> OrderBook::best_bid() const
{
    if (bids_.empty())
    {
        return std::nullopt;
    }

    return bids_.begin() -> first;
}

std::optional<Price> OrderBook::best_ask() const
{
    if (asks_.empty())
    {
        return std::nullopt;
    }

    return asks_.begin() -> first;
}

std::size_t OrderBook::bid_level_count() const
{
    return bids_.size();
}


std::size_t OrderBook::ask_level_count() const
{
    return asks_.size();
}

bool OrderBook::would_cross(const Order& order) const
{
    if (order.side == Side::Buy)
    {
        const auto ask = best_ask();
        
        // we use has_value as best ask/bid returns a std::optional so it could be empty
        return ask.has_value() && 
            order.price >= *ask;
    }

    // for a sell order
    const auto bid = best_bid();
    return bid.has_value() &&
        order.price <= *bid;
}

std::optional<Price> // return type
OrderBook::execution_price(const Order& order) const
{
    //checks if the spread is crossed
    if (!would_cross(order))
    {
        return std::nullopt;
    }

    if (order.side == Side::Buy)
    {
        return best_ask();
    }

    return best_bid();
}

std::optional<Trade> // return type
OrderBook::execute_one(Order& incoming)
{
    if (!would_cross(incoming))
    {
        return std::nullopt;  // no trade occurs
    }

    if (incoming.side == Side::Buy){
        auto ask_level = asks_.begin();

        // implements the time part of price-time priority
        auto& resting_order = ask_level -> second.front();

        const Quantity traded_quantity =
            std::min(incoming.quantity, resting_order.quantity);
        
        Trade trade{
            incoming.id,
            resting_order.id,
            resting_order.price,
            traded_quantity
        };

        incoming.quantity -= traded_quantity;
        resting_order.quantity -= traded_quantity;

        // removing a filled resting order
        if (resting_order.quantity == 0)
        {
            // price-time priority, getting rid of the earliest order (deque front)
            ask_level -> second.pop_front();

            // if the price level is empty, then useless
            if (ask_level->second.empty())
            {
                asks_.erase(ask_level);
            }
        }
        return trade;

    }

    // if the incoming order is a sell
    auto bid_level = bids_.begin();

    auto& resting_order = bid_level -> second.front();

    const Quantity traded_quantity =
        std::min(incoming.quantity, resting_order.quantity);

    Trade trade{
        resting_order.id,
        incoming.id,
        resting_order.price,
        traded_quantity
    };

    incoming.quantity -= traded_quantity;
    resting_order.quantity -= traded_quantity;

    if (resting_order.quantity == 0){
        bid_level -> second.pop_front();

        if (bid_level -> second.empty()){
            bids_.erase(bid_level);
        }
    }

    return trade;
}

// we dont use std::optional anymore as it can store at most one object
std::vector<Trade>
OrderBook::execute(Order& incoming)
{
    // initializing our trades array
    std::vector<Trade> trades;

    while(incoming.quantity > 0 
        && would_cross(incoming))
    {
        const auto trade = execute_one(incoming);

        if (!trade){
            break;
        }

        trades.push_back(*trade);
    }
    return trades;
}

std::vector<Trade>
OrderBook::submit(Order incoming)
{
    // making sure our order id and quantity are both non-zero
    validate_order(incoming);
    
    // fully executes the trades till its either all done or the order is passive
    auto trades = execute(incoming);

    if (incoming.quantity > 0){
        add_order(incoming);
    }

    return trades;
}

void OrderBook::validate_order(const Order& order) const
{
    if (order.id == 0){
        throw std::invalid_argument(
            "Order ID must be non-zero"
        );
    }

    if (order.quantity == 0)
    {
        throw std::invalid_argument(
            "Order quantity must be non-zero"
        );
    }

}

}

