#include <iostream>
#include <stdexcept>

#include "exchange/order_book.hpp"

// added 3 buy orders but 2 bid price levels as 2 orders have same price
int main()
{

    exchange::OrderBook cancel_book;

    (void)cancel_book.submit({
        10,
        10125,
        100,
        exchange::Side::Buy
    });

    (void)cancel_book.submit({
        11,
        10120,
        50,
        exchange::Side::Buy

    });

    std::cout << "Best bid before cancel: "
          << *cancel_book.best_bid()
          << '\n';
    

    const bool cancelled = cancel_book.cancel(10);

    std::cout << "Cancelled #10: "
            << std::boolalpha
            << cancelled
            << '\n';

    std::cout << "Best bid after cancel: "
            << *cancel_book.best_bid()
            << '\n';


    std::cout << "Cancel #999: "
          << cancel_book.cancel(999)
          << '\n';

}

