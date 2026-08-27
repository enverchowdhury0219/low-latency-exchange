#include <iostream>
#include <cstdint>
#include "exchange/order.hpp"

struct ExampleA
{
    std::uint8_t a;
    std::uint64_t b;
};

struct ExampleB
{
    std::uint64_t b;
    std::uint8_t a;
};

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
    std::cout <<"Order Alignment: " << alignof(order) << " bytes\n";

    std::cout << "ExampleA size: " << sizeof(ExampleA) << " bytes\n";
    std::cout << "ExampleB size: " << sizeof(ExampleB) << " bytes\n";

    return 0;
}

