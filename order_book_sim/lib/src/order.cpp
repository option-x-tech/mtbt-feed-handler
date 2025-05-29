#include "order.hpp"

bool operator<(const Order &lhs, const Order &rhs)
{
    return lhs.price < rhs.price;
}