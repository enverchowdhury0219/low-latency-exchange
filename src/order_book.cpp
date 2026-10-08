#include "exchange/order_book.hpp"
#include <algorithm> // for std::min
#include <stdexcept> // for throwing exceptions
#include <utility>


namespace exchange
{

void OrderBook::add_order(const Order& order)
{
    if (order.side == Side::Buy)
    {
        auto& orders = bids_[order.price];

        auto order_it = orders.insert(
            orders.end(),
            order
        );

        // inserts order at the end - preserves time priority
        // also returns interator pointing to inserted order
        order_locations_.insert({
            order.id,
            {
                order.side,
                order.price,
                order_it
            }
        });

        return;
    }
    
    auto& orders = asks_[order.price];

    auto order_it = orders.insert(
        orders.end(),
        order
    );

    order_locations_.insert({
            order.id,
            {
                order.side,
                order.price,
                order_it
            }
        });
    
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
            // we do this first as .pop() would remove that order object whose id we need to delete
            order_locations_.erase(resting_order.id);
            
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
        
        order_locations_.erase(resting_order.id);
        
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

SubmitResult
OrderBook::submit(Order incoming)
{
   const RejectReason reject_reason = 
    validate_order(incoming);

    if (reject_reason != RejectReason::None)
    {
        return {
            false,
            reject_reason,
            {}
        };
    }

    auto trades = execute(incoming);

    if (incoming.quantity > 0)
    {
        add_order(incoming);
    }

    return {
        true,
        RejectReason::None,
        std::move(trades) // essentially 'steals' the value of trades and stores it in this SubmitResult
    };


}

RejectReason
OrderBook::validate_order(const Order& order) const
{
    if (order.id == 0)
    {
        return RejectReason::InvalidOrderId;
    }

    if (order.quantity == 0)
    {
        return RejectReason::InvalidQuantity;
    }

    if (order_locations_.find(order.id) != order_locations_.end())
    {
        return RejectReason::DuplicateOrderId;
    }

    return RejectReason::None;
}

bool OrderBook::cancel(OrderId id)
{

    const auto location_it = order_locations_.find(id);

    if (location_it == order_locations_.end())
    {
        return false; // it is not in the book
    }

    const OrderLocation location = location_it -> second;

    if (location.side == Side::Buy)
    {
        auto level = bids_.find(location.price);

        if (level == bids_.end())
        {
            return false; // not in the book so cant cancel
        }

        level -> second.erase(location.order);
        order_locations_.erase(location_it);

        if (level -> second.empty())
        {
            bids_.erase(level);
        }

        return true;
    }


    auto level = asks_.find(location.price);
    
    if (level == asks_.end())
    {
        return false;
    }
    
    level -> second.erase(location.order);
    order_locations_.erase(location_it);

    if (level -> second.empty())
    {
        asks_.erase(level);
    }

    return true;
    
}

ReplaceResult
OrderBook::replace(
    OrderId id,
    Price new_price,
    Quantity new_quantity
)
{
    if (new_quantity == 0)
    {
        // now we return ReplaceResults
        return {
            false,
            RejectReason::InvalidQuantity,
            {}
        };

    }

    const auto location_it =
        order_locations_.find(id);

    if (location_it == order_locations_.end())
    {
        return {
            false,
            RejectReason::UnknownOrderId,
            {}
        };
    }

    // copy the old order before cancelling it
    const Order old_order =
        *(location_it -> second.order);

    if (!cancel(id))
    {
        return {
            false,
            RejectReason::UnknownOrderId,
            {}
        };
    }

    // creates a newly arriving order with same id and side
    Order replacement{
        id,
        new_price,
        new_quantity,
        old_order.side // we copied the old order's side before cancelling
    };

    auto submit_result = submit(replacement);

    if (!submit_result.accepted)
    {
        return {
        false,
        submit_result.reject_reason,
        {}
        };
    }

    return {
        true,
        RejectReason::None,
        std::move(submit_result.trades)
    };

}

}
