#ifndef P1_ORDER_
#define P1_ORDER_

class Order
{
public:
    long timestamp;
    double orderId;
    int price;
    int quantity;

    Order(long timestamp, double orderId, int price, int quantity) : timestamp(timestamp), orderId(orderId), price(price), quantity(quantity) {}
    Order(const Order &order) : orderId(order.orderId), price(order.price), quantity(order.quantity) {}
    Order(){}
};

bool operator<(const Order &lhs, const Order &rhs);

#endif // P1_ORDER_