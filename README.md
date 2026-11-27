# Low-Latency Exchange & Market-Making Engine

an electronic exchange in c++ and event-driven market-making simulator built to explore low-latency systems, market microstructure, matching-engine design and more.

this implements a price-time-priority limit order book, exchange event layer, market-data interface, inventory-aware market maker, PnL accounting, automated tests, and performance benchmarks.

---

## Overview

this models the core lifecycle of an electronic market:

```text
Market Orders / Limit Orders
          ↓
       Exchange
          ↓
    Limit Order Book
          ↓
     Matching Engine
          ↓
     Exchange Events
          ↓
     Market Maker
          ↓
Inventory / Risk / PnL
          ↓
       Requote
```

the exchange handles order validation, matching, cancellation, replacement, and market-data generation.

The market maker observes the order book, estimates fair value, submits bid and ask quotes, reacts to fills, manages inventory risk, and tracks realized and unrealized PnL.

---

## Core Features

### Limit Order Book

- price-time priority matching
- separate bid and ask price levels
- FIFO ordering within each price level
- integer-based prices for exact financial comparisons
- partial fills
- multi-order and multi-level execution
- resting remainder handling
- best bid and best ask retrieval
- aggregate quantity at the top of book

### Order Management

- submit new orders
- cancel resting orders
- replace existing orders
- duplicate order-ID detection
- order validation and explicit rejection reasons
- direct order-location indexing for efficient cancellation
- stable iterators for resting orders
- cached aggregate quantity at each price level

### Exchange Event System

the exchange we made avoids strongly typed events using `std::variant`, including:

- `OrderAccepted`
- `OrderRejected`
- `OrderCancelled`
- `CancelRejected`
- `OrderReplaced`
- `ReplaceRejected`
- `Trade`
- `TopOfBookUpdate`

this separates the internal matching engine from the external behavior observed by trading strategies.

### Market Data

the exchange exposes:

- best bid
- best ask
- best bid quantity
- best ask quantity
- top-of-book updates

these values are used directly by the trading strategy.

---

## Market-Making Strategy

project includes an event-driven market maker that generates and manages its own bid and ask orders.

### Fair Value

2 fair-value estimators are implemented.

#### Midpoint

```text
midpoint = best bid + (best ask - best bid) / 2
```

#### Microprice

incorporates top-of-book quantity imbalance:

```text
microprice =
(ask_price × bid_quantity + bid_price × ask_quantity)
-----------------------------------------------------
             bid_quantity + ask_quantity
```

a larger bid quantity shifts estimated fair value toward the ask, while a larger ask quantity shifts it toward the bid.

### Quote Generation

quotes are generated around estimated fair value:

```text
bid = adjusted fair value - half spread
ask = adjusted fair value + half spread
```

the strategy generates unique order IDs and tracks active bid and ask orders.

### Inventory Skew

quotes are adjusted according to the size and direction of the strategy's inventory.

long position - shifts the quote downward, making the strategy less aggressive when buying and more aggressive when selling.

short position - shifts the quote upward.

```text
inventory adjustment =
inventory × inventory skew / inventory unit
```

this provides a soft mechanism for controlling inventory exposure.

### Position Limits

strategy also implements a hard maximum inventory constraint.

before submitting an order, it checks whether a complete fill could push inventory beyond the configured limit.

this creates 2 layers of risk control:

```text
inventory skew
→ soft pricing-based risk management

position limit
→ hard exposure boundary
```

---

## Fill and Order Lifecycle

market maker tracks active order IDs and remaining quantities.

supports:

- partial fills
- full fills
- removal of fully filled active orders
- automatic inventory updates
- cancel-and-requote behavior
- exchange-generated trade event processing

typical strategy cycle:

```text
observe market
      ↓
estimate fair value
      ↓
generate bid / ask
      ↓
submit to exchange
      ↓
receive trade events
      ↓
update inventory
      ↓
cancel existing quotes
      ↓
recalculate fair value
      ↓
requote
```

the strategy removes its own active quotes before recalculating market-derived fair value so that its own displayed liquidity does not distort its signal.

---

## PnL Accounting

market maker tracks:

- cash
- inventory
- average entry price
- realized PnL
- unrealized PnL
- total mark-to-market PnL

for a mark price:

```text
total PnL
=
realized PnL + unrealized PnL
```

equivalent cash-and-inventory relationship:

```text
total PnL
=
cash + inventory × mark price
```

a completed simulated market-making round trip produces:

```text
Buy  100 @ 10100
Sell 100 @ 10108

Final inventory: 0
Realized PnL: 800
```

PnL is expressed in price-tick × quantity units.

---

## Exchange Statistics

the exchange records operational statistics including:

- orders submitted
- orders accepted
- orders rejected
- trades executed
- total traded quantity
- successful and rejected cancellations
- successful and rejected replacements

completed simulation produces:

```text
Orders submitted: 8
Orders accepted: 8
Orders rejected: 0
Trades executed: 2
Traded quantity: 200
Cancels accepted: 1
```

---

## Project Architecture

```text
include/exchange/
├── order.hpp
├── trade.hpp
├── order_result.hpp
├── order_book.hpp
├── event.hpp
├── exchange.hpp
├── stats.hpp
└── market_maker.hpp

src/
├── main.cpp
├── order_book.cpp
├── exchange.cpp
└── market_maker.cpp

tests/
├── order_book_tests.cpp
├── exchange_tests.cpp
└── market_maker_tests.cpp

benchmarks/
└── order_book_benchmark.cpp
```

### Main Components

**OrderBook**

owns resting orders, price levels, matching logic, cancellation, replacement, and book state.

**Exchange**

provides the external order interface and converts order-book behavior into exchange events and market-data updates.

**MarketMaker**

consumes market information and exchange events, estimates fair value, generates quotes, manages inventory, enforces risk limits, and tracks PnL.

---

## Data Structure Design

The book uses:

- `std::map` for sorted bid and ask price levels
- `std::list` for FIFO orders at each price
- `std::unordered_map` for direct order-ID lookup

each price level stores:

```cpp
struct PriceLevel
{
    OrdersAtPrice orders;
    Quantity total_quantity{0};
};
```

the cached aggregate quantity allows constant-time retrieval of displayed quantity at the best price.

the order-location index stores the side, price, and stable list iterator for each live order, allowing direct cancellation without scanning every order in the book.

---

## Correctness Invariants

The implementation maintains several important invariants:

- Live order IDs are unique.
- Order IDs must be nonzero.
- Order quantities must be positive.
- Resting orders follow price-time priority.
- Execution occurs at the resting order's price.
- Fully filled orders are removed from the location index.
- Replacement loses previous queue priority.
- The resting book is not crossed after processing.
- Cached price-level quantity equals the sum of resting quantities at that level.
- Invalid replacement requests do not mutate the original order.

A regression test specifically protects cached quantity consistency after cancellation.

---

## Performance

Benchmarks were run using an optimized Release build with AppleClang on macOS. Each workload contains 100,000 operations.

| Workload | Average Latency | Throughput |
|---|---:|---:|
| Resting order insertion | ~59 ns/op | ~16.9M ops/sec |
| Matching | ~53 ns/op | ~19.0M ops/sec |
| Cancellation | ~23 ns/op | ~43.2M ops/sec |

Batched resting-order latency measurements produced approximately:

```text
p50: 33–35 ns/op
p95: 37–42 ns/op
p99: 44–93 ns/op
```

Single-operation measurements showed timer quantization from `std::chrono::steady_clock` of approximately 42 ns at higher percentiles. Batched measurements were therefore used to reduce measurement overhead.

These results are synthetic microbenchmarks intended to compare implementation paths and data-structure behavior. They should not be interpreted as production exchange or network latency.

---

## Build

Requirements:

- C++20-compatible compiler
- CMake 3.20+

Configure and build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Run the test suite:

```bash
ctest --test-dir build --output-on-failure
```

---

## Run the Simulation

```bash
./build/exchange_sim
```

Example final strategy output:

```text
Initial microprice: 10105

After bid fill:
  Inventory: 100
  Cash: -1010000

Final strategy state:
  Inventory: 0
  Cash: 800
  Realized PnL: 800
  Unrealized PnL: 0
  Total PnL: 800
```

---

## Run Optimized Benchmarks

Configure the Release build:

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

Run:

```bash
./build-release/order_book_benchmark
```

---

## Testing

The project includes dedicated test executables for:

- Order-book behavior
- Exchange event behavior
- Market-making strategy behavior

Coverage includes:

- Price-time priority
- Partial fills
- Multi-level fills
- Cancellation
- Replacement
- Duplicate IDs
- Invalid orders
- Top-of-book state
- Aggregate price-level quantities
- Exchange events
- Market-maker order generation
- Partial and full strategy fills
- Inventory skew
- Microprice calculation
- Quote refresh
- Position limits
- Cash accounting
- Realized and unrealized PnL
- End-to-end fill processing
- Cancellation quantity regression

---

## Engineering Focus

This project was built around several principles:

1. **Correctness before optimization** — matching semantics and invariants were established before performance work.
2. **Measurement before optimization** — performance decisions were evaluated using Release-build microbenchmarks.
3. **Explicit state ownership** — matching, exchange behavior, and strategy logic are separated into distinct components.
4. **Event-driven design** — trading logic reacts to exchange-generated events rather than modifying book state directly.
5. **Risk-aware strategy behavior** — inventory skew and hard position limits are integrated into order generation.
6. **Testable abstractions** — core behavior is covered by unit and integration tests so internal implementation details can evolve safely.

---

## Project Status

**Complete.**

The project implements a fully connected simulated electronic exchange and market-making system in C++20, including matching, order management, market data, event processing, strategy logic, inventory controls, PnL accounting, testing, and performance measurement.
