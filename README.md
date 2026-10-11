# Low-Latency Exchange

a work-in-progress C++ electronic exchange and limit order book built as a hands-on exploration of low-latency systems and algorithmic trading infrastructure.

## Current Features

- limit order book with bid/ask price levels
- price-time priority
- partial and multi-level fills
- resting order management
- order cancellation and replacement
- duplicate-order protection
- explicit submit/reject results
- automated order book tests with CTest

## Current Architecture

The engine currently uses:

- `std::map` for sorted price levels
- `std::list` for stable order iterators
- `std::unordered_map` for direct order lookup
- Integer-based prices to avoid floating-point precision issues

These structures are intentionally being treated as a correct baseline before profiling and low-latency optimization.

## Goal

The goal is to progressively build from a correct single-threaded matching engine into a more complete electronic trading system while learning and measuring the engineering tradeoffs behind:

- market microstructure
- matching engines
- memory layout and cache locality
- data structure design
- latency and throughput
- market data
- risk controls
- market-making strategies

## Status

**Work in progress.**

current focus: completing the exchange/order lifecycle and preparing the engine for event handling, market-data output, benchmarking, and performance optimization.
