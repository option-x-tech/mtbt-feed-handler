#ifndef P1_LEVEL_OrderBook_
#define P1_LEVEL_OrderBook_

#include "read_data.hpp"

class Level; 

class Order
{
public:
    long timestamp;
    int price;
    int quantity;
    std::list<Level>::iterator order_iterator;
    Order(long timestamp_, int price_, int quantity_, std::list<Level>::iterator order_iterator_) : timestamp(timestamp_), price(price_), quantity(quantity_), order_iterator(order_iterator_) {}
    Order(const Order &order) : timestamp(order.timestamp), price(order.price), quantity(order.quantity), order_iterator(order.order_iterator) {}
    Order() {};
};

class Level
{
public:
    // Total quantity of these orders
    int total_quantity;
    // Prive of this level
    int price;

    bool empty();
    size_t size();

    Level(int quantity_, int price_) : total_quantity(quantity_), price(price_)
    {
    }
};

class LevelOrderBook
{
    // Vector of Levels sorted on price
    std::list<Level> sell_order_book; // Low to High
    std::list<Level> buy_order_book;  // High to Low

    // Unordered map of orderId to its order metadata
    std::unordered_map<double, Order> buy_order_level;
    std::unordered_map<double, Order> sell_order_level;

    int get_best_bid();
    int get_best_ask();

    OrderMessage active_order_message;

public:
    LevelOrderBook()
    {
        active_order_message.quantity = 0;
    }

    void process_trade_message(TradeMessage &trade_message);

    void process_new_order_message(OrderMessage &orderMessage);
    void process_modify_order_message(OrderMessage &orderMessage);
    void process_cancel_order_message(OrderMessage &orderMessage);
    void check_and_insert_active_order();

    void print_statistics(int token, long time_stamp);
    void print_ask_order_book();
    void print_bid_order_book();
    void print_num_orders();
};

#endif // P1_LEVEL_OrderBook