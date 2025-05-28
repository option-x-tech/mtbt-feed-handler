#ifndef P1_ORDER_BOOK_
#define P1_ORDER_BOOK_

#include "order.hpp"
#include "read_data.hpp"

class OrderBook
{
    std::map<double, Order> buyOrders;
    std::map<double, Order> sellOrders;

    std::multiset<Order> buyOrderBook;
    std::multiset<Order> sellOrderBook;

    int get_best_bid();
    int get_best_ask();

public:
    void process_order_message(OrderMessage orderMessage);
    void process_trade_message(TradeMessage tradeMessage);

    void print_statistics(long time_stamp); 
    void print_ask_order_book(); 
    void print_bid_order_book(); 
    void print_num_orders(); 
};

#endif // P1_ORDER_BOOK_