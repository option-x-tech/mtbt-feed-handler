#ifndef P1_LEVEL_OrderBook_
#define P1_LEVEL_OrderBook_

#include "order.hpp"
#include "read_data.hpp"

class Level
{
    // Map of orderId to order
    int quantity;

public:
    Level();
    std::unordered_map<double, Order> orders;
    bool empty();
    size_t size();
    int highest() const;
    int lowest() const;
    void add_order(Order order);
    bool find_order(double orderId);
    void erase_order(Order order);
    bool update_order(double orderID, int trade);
    int print_quantity();
};

class LevelOrderBook
{
    // Map of levels sorted based on thier prices
    std::map<int, Level> buy_order_book;
    std::map<int, Level> sell_order_book;

    // Unordered map of orderId to its level (used in handling trade messages)
    std::unordered_map<double, int> buy_order_level;
    std::unordered_map<double, int> sell_order_level;

    int get_best_bid();
    int get_best_ask();

    OrderMessage active_order_message;

public:
    LevelOrderBook()
    {
        active_order_message.quantity = 0;
    }

    // Order message
    void process_order_message(OrderMessage order_message);
    // Trade message
    void process_trade_message(TradeMessage trade_message);

    void process_new_order_message(OrderMessage orderMessage);
    void process_modify_order_message(OrderMessage orderMessage);
    void process_cancel_order_message(OrderMessage orderMessage);
    void check_and_insert_active_order();

    void print_statistics(int token, long time_stamp);
    void print_ask_order_book();
    void print_bid_order_book();
    void print_num_orders();
};

#endif // P1_LEVEL_OrderBook