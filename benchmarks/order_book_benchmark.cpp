#include <chrono>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <vector>

#include "exchange/order_book.hpp"


// benchmarks to test out 3 hot paths:
// 1. resting order insertion
// 2. crossing/matching
// 3. cancellation

// we then ran these multiple times to get a better idea of how fast our orderbook can perform
// certain operations at scale

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

    // setup is intentionally outside the timed region
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

    // once again, the setup is not timed.
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

// helper function to print percentiles
void print_percentiles(
    std::vector<std::int64_t>& latencies
)
{
    std::sort(latencies.begin(), latencies.end());

    const auto p50 =
        latencies[latencies.size() * 50 / 100];

    const auto p95 =
        latencies[latencies.size() * 95 / 100];

    const auto p99 =
        latencies[latencies.size() * 99 / 100];

    std::cout << "p50: " << p50 << " ns\n";
    std::cout << "p95: " << p95 << " ns\n";
    std::cout << "p99: " << p99 << " ns\n";
}

// adding our percentile benchmark
// sample results interpretation:
// p50: 80 ns
// p95: 110 ns
// p99: 160 ns
// p50: half of measured orders completed within 80 ns
// p95: 95% completed within 110 ns
// p99: 99% completed within 160 ns

void benchmark_resting_insert_latency()
{
    exchange::OrderBook book;

    std::vector<std::int64_t> latencies;
    latencies.reserve(iterations);

    for (std::uint64_t i = 1; i <= iterations; ++i)
    {
        const auto start = Clock::now();

        (void)book.submit({
            i,
            10000 + static_cast<exchange::Price>(i % 100),
            100,
            exchange::Side::Buy
        });

        const auto end = Clock::now();

        const auto elapsed =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                end - start
            ).count();

        latencies.push_back(elapsed);
    }

    std::cout << "\nResting insert latency percentiles\n";
    print_percentiles(latencies);
}

// adding a benchmark to measure the the measurement overhead itself
// so essentially seeing how much time the clock actually takes itself to
// better recalibrate the actual time orders take
void benchmark_clock_overhead()
{
    std::vector<std::int64_t> latencies;
    latencies.reserve(iterations);

    for (std::uint64_t i = 0; i < iterations; ++i)
    {
        const auto start = Clock::now();
        const auto end = Clock::now();

        const auto elapsed =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                end - start
            ).count();

        latencies.push_back(elapsed);
    }

    std::cout << "\nClock measurement overhead\n";
    print_percentiles(latencies);
}

// to offset the measurement overhead as much as we can, we can measure orders
// in batches

void benchamrk_batched_insert_latency()
{
    constexpr std::uint64_t batch_size = 64;
    constexpr std::uint64_t batch_count =
        iterations / batch_size;

    exchange::OrderBook book;

    std::vector<std::int64_t> latencies;
    latencies.reserve(batch_count);

    std::uint64_t order_id = 1;

    for (std::uint64_t batch = 0;
            batch < batch_count;
            ++batch)
    {
        const auto start = Clock::now();

        
    }
}

int main()
{
    benchmark_resting_inserts();
    benchmark_matching();
    benchmark_cancellation();

    benchmark_resting_insert_latency();
    benchmark_clock_overhead();

    return 0;
}