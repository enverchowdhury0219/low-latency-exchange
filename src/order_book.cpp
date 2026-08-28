#include "exchange/order_book.hpp"

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

}