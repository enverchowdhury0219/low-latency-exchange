cmake_minimum_required(VERSION 3.20)

project(
    low_latency_exchange
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
    exchange_sim
    src/main.cpp
)

target_compile_options(exchange_sim PRIVATE -Wall -Wextra -Wpedantic)