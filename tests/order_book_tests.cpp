// created first test file

#include <iostream>
#include <stdexcept>
#include <string>

#include "exchange/order_book.hpp"

void expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void test_best_bid_and_ask()
{

    
}