#include <chrono>
#include <cstdint>
#include <iostream>

#include "exchange/order_book.hpp"

// our first benchmark - which is the process of measuring the exact performance/execution time
// of the code i.e. a highly precise digital stopwatch timing our CPU

// we are testing time taken to submit 100,000 orders
// and we do time (nanoseconds) / # orders to get ns/order
int main()
{
    exchange::OrderBook book;

    // uint64_t - exact width unsigned 64 bit integer defined in <cstdint>
    // constexpr - specifier telling compiler that variable can potentially be evaluated at compile time
    constexpr std::uint64_t iterations = 100000;

    const auto start =
        std::chrono::steady_clock::now();

    for (std::uint64_t i = 1; i <= iterations; i++)
    {
        (void)book.submit({
            i,
            // static_cast - compile-time type conversion to safely transform underlying tick count
            // of a duration into a uint64_t
            10000 + static_cast<exchange::Price>(i % 100),
            100,
            exchange::Side::Buy
        });
    }

    // chrono - standard c++ library for date and time uses
    // steady_clock - monotinic clock whose time points are guarateed to never move backward,
    // so ideal here for benchmarking.
    // steady_clock::now() - static member function returning a time_point representing current time of steady clock
    const auto end =
        std::chrono::steady_clock::now();

        // duration_cast<ToDuration> - explicit conversion function returning a required when converting to
        // a duration to a different precision (scaling up/down) to prevent unintended data loss
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