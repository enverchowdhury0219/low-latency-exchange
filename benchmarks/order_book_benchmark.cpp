#include <chrono>
#include <cstdint>
#include <iostream>

#include "exchange/order_book.hpp"

using Clock = std::chrono::steady_clock;

constexpr std::uint64_t iterations = 100000;

void print_result(
    const char* name,
    std::int64_t elapsed_ns
)
{
    const double ns_per_operation =
        static_cast<double>(elapsed_ns) / iterations;

    const double operations_per_second =
        1'000'000'000.0 / ns_per_operation;

    std::cout << "\n" << name << '\n';
    std::cout << "Operations: " << iterations << '\n';
    std::cout << "Total: " << elapsed_ns << " ns\n";
    std::cout << "Average: "
              << ns_per_operation
              << " ns/op\n";
    std::cout << "Throughput: "
              << operations_per_second
              << " ops/sec\n";
}

void benchmark_resting_inserts()
{
    exchange::OrderBook book;

    const auto start = Clock::now();

    for (std::uint64_t i = 1; i <= iterations; ++i)
    {
        (void)book.submit({
            i,
            10000 + static_cast<exchange::Price>(i % 100),
            100,
            exchange::Side::Buy
        });
    }

    const auto end = Clock::now();

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            end - start
        ).count();

    print_result("Resting inserts", elapsed);
}

void benchmark_matching()
{
    exchange::OrderBook book;

    // Setup is intentionally outside the timed region.
    for (std::uint64_t i = 1; i <= iterations; ++i)
    {
        (void)book.submit({
            i,
            10100,
            100,
            exchange::Side::Sell
        });
    }

    const auto start = Clock::now();

    for (std::uint64_t i = 1; i <= iterations; ++i)
    {
        (void)book.submit({
            iterations + i,
            10100,
            100,
            exchange::Side::Buy
        });
    }

    const auto end = Clock::now();

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            end - start
        ).count();

    print_result("Matching", elapsed);
}

void benchmark_cancellation()
{
    exchange::OrderBook book;

    // Again, setup is not timed.
    for (std::uint64_t i = 1; i <= iterations; ++i)
    {
        (void)book.submit({
            i,
            10000,
            100,
            exchange::Side::Buy
        });
    }

    const auto start = Clock::now();

    for (std::uint64_t i = 1; i <= iterations; ++i)
    {
        (void)book.cancel(i);
    }

    const auto end = Clock::now();

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            end - start
        ).count();

    print_result("Cancellation", elapsed);
}

int main()
{
    benchmark_resting_inserts();
    benchmark_matching();
    benchmark_cancellation();

    return 0;
}