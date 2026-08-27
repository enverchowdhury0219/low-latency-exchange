#include <iostream>
#include <cstdint>
#include "exchange/order.hpp"


int main()
{
    exchange::Order order{
        1,
        10125,
        100,
        exchange::Side::Buy
    };

    std::cout << "Exchange simulator initialized.\n";
    std::cout <<"Order Size: " << sizeof(order) << " bytes\n";

    return 0;
}

