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

add_library(
    exchange_core
    src/order_book.cpp
)

target_include_directories(
    exchange_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_options(
    exchange_core
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

add_executable(
    exchange_sim
    src/main.cpp
)

target_link_libraries(
    exchange_sim
    PRIVATE
        exchange_core
)

target_compile_options(
    exchange_sim
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

enable_testing()

add_executable(
    order_book_tests
    tests/order_book_tests.cpp
)

target_link_libraries(
    order_book_tests
    PRIVATE
        exchange_core
)

target_compile_options(
    order_book_tests
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

add_test(
    NAME order_book_tests
    COMMAND order_book_tests
)