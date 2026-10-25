#include <chrono>
#include <cstdint>
#include <iostream>

#include "exchange/order_book.hpp"

int main()
{
    exchange::OrderBook book;

    constexpr std::uint64_t iterations = 100000;

    const auto start =
        std::chrono::steady_clock::now();

    for (std::uint64_t i = 1; i <= iterations; i++)
    {
        (void)book.submit({
            i,
            10000 + static_cast<exchange::Price>(i % 100),
            100,
            exchange::Side::Buy
        });
    }

    const auto end =
        std::chrono::steady_clock::now();

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            end - start
        ).count();

    const double ns_per_order = 
        static_cast<double>(elapsed) / iterations;

    std::cout << "Orders: " << iterations << '\n';
    std::cout << "Total time: " << elapsed << " ns\n";
    std::cout << "Average: " << ns_per_order
              << " ns/order\n";

    return 0;
        
}