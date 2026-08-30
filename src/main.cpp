#include <iostream>

#include "exchange/order_book.hpp"

// added 3 buy orders but 2 bid price levels as 2 orders have same price
int main()
{
   exchange::OrderBook sweep_book;

    sweep_book.add_order({
        1,
        10130,
        50,
        exchange::Side::Sell
    });

    sweep_book.add_order({
        2,
        10130,
        75,
        exchange::Side::Sell
    });

    sweep_book.add_order({
        3,
        10135,
        100,
        exchange::Side::Sell
    });

    sweep_book.add_order({
        4,
        10145,
        200,
        exchange::Side::Sell
    });

    exchange::Order sweep_buy{
    20,
    10140,
    200,
    exchange::Side::Buy
    };

    const auto trades = sweep_book.execute(sweep_buy);

    std::cout << "\nSweep produced "
          << trades.size()
          << " trades\n";

    for (const auto& trade : trades)
    {
        std::cout
            << "Buyer #" << trade.buy_order_id
            << " matched Seller #" << trade.sell_order_id
            << " | Price: " << trade.price
            << " | Quantity: " << trade.quantity
            << '\n';
    }

    std::cout << "Incoming remaining quantity: "
            << sweep_buy.quantity
            << '\n';

    std::cout << "Best ask after sweep: "
            << *sweep_book.best_ask()
            << '\n';


}

