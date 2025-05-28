#include "level_order_book.hpp"

Level::Level() : quantity(0) {}

bool Level::empty()
{
    return orders.empty();
}

size_t Level::size()
{
    return orders.size();
}

int Level::highest() const
{
    int high = INT32_MIN;
    for (auto it : orders)
    {
        high = std::max(high, it.second.price);
    }
    return high;
}

int Level::lowest() const
{
    int low = INT32_MAX;
    for (auto it : orders)
    {
        low = std::min(low, it.second.price);
    }
    return low;
}

void Level::add_order(Order order)
{
    assert(orders.find(order.orderId) == orders.end());
    orders[order.orderId] = order;
}

bool Level::find_order(double orderId)
{
    return orders.find(orderId) != orders.end();
}

void Level::erase_order(Order order)
{
    assert(orders.find(order.orderId) != orders.end());
    orders.erase(order.orderId);
}

bool Level::update_order(double orderID, int trade)
{
    orders[orderID].quantity -= trade;
    assert(orders[orderID].quantity >= 0);
    if (orders[orderID].quantity == 0)
    {
        orders.erase(orderID);
        return true;
    }
    return false;
}

int Level::print_quantity()
{
    int sum = 0;
    for (auto it : orders)
    {
        sum += it.second.quantity;
    }
    return sum;
}

int LevelOrderBook::get_best_bid()
{
    if (!buy_order_book.empty())
        return buy_order_book.rbegin()->second.highest();
    return INT32_MIN;
}

int LevelOrderBook::get_best_ask()
{
    if (!sell_order_book.empty())
        return sell_order_book.begin()->second.lowest();
    return INT32_MAX;
}

void LevelOrderBook::process_new_order_message(OrderMessage orderMessage)
{
    Order order(orderMessage.timestamp, orderMessage.orderID, orderMessage.price, orderMessage.quantity);

    // Check if active order exists (It should not)
    assert(active_order_message.quantity == 0);

    // If both the Buy and Ask orderbooks are not empty then check if they are not crossing
    if (!buy_order_book.empty() && !sell_order_book.empty())
    {
        assert(get_best_bid() < get_best_ask());
    }

    // Check for active order condition
    if (orderMessage.orderType == 'B')
    {
        if (get_best_ask() <= order.price)
        {
            active_order_message = orderMessage;
            return;
        }
    }
    if (orderMessage.orderType == 'S')
    {
        if (get_best_bid() >= order.price)
        {
            active_order_message = orderMessage;
            return;
        }
    }

    if (orderMessage.orderType == 'B')
    {
        // First check if a level is present for this message
        if (buy_order_book.find(orderMessage.price) != buy_order_book.end())
        {
            buy_order_book[orderMessage.price].add_order(order);
        }
        // If such a level is not present then create a new level and insert it in the order book
        else
        {
            // Create a level add order in it and add level in order book
            Level level;
            level.add_order(order);
            buy_order_book[orderMessage.price] = level;
        }
        buy_order_level[orderMessage.orderID] = orderMessage.price;
    }

    if (orderMessage.orderType == 'S')
    {
        // First check if a level is present for this message
        if (sell_order_book.find(orderMessage.price) != sell_order_book.end())
        {
            sell_order_book[orderMessage.price].add_order(order);
        }
        // If such a level is not present then create a new level ans insert it in the order book
        else
        {
            // Create a level add order in it and add level in order book
            Level level;
            level.add_order(order);
            sell_order_book[orderMessage.price] = level;
        }
        sell_order_level[orderMessage.orderID] = orderMessage.price;
    }
}

void LevelOrderBook::check_and_insert_active_order()
{
    // An active order is present for the moment
    if (active_order_message.quantity != 0)
    {
        // Remove the existing active order and pass the active order as a new message
        OrderMessage new_active_order_message = active_order_message;
        active_order_message.quantity = 0;
        process_new_order_message(new_active_order_message);
    }
}

void LevelOrderBook::process_cancel_order_message(OrderMessage orderMessage)
{
    Order order(orderMessage.timestamp, orderMessage.orderID, orderMessage.price, orderMessage.quantity);

    // If both the Buy and Ask orderbooks are not empty then check if they are not crossing
    if (!buy_order_book.empty() && !sell_order_book.empty())
    {
        assert(get_best_bid() < get_best_ask());
    }

    // If an active order is present
    if (active_order_message.quantity != 0)
    {
        // Check if the cancel message is for the active order
        if (order.orderId == active_order_message.orderID)
        {
            active_order_message.quantity = 0;
            return;
        }
    }

    // If an active order is not present or the cancel message is not for it
    if (orderMessage.orderType == 'B')
    {
        if (buy_order_level.find(orderMessage.orderID) == buy_order_level.end())
        {
            orderMessage.printValues();
            return;
        }

        auto price = buy_order_level[orderMessage.orderID];
        buy_order_book[price].erase_order(order);

        if (buy_order_book[price].empty())
        {
            buy_order_book.erase(price);
        }
        buy_order_level.erase(orderMessage.orderID);
    }
    if (orderMessage.orderType == 'S')
    {
        if (sell_order_level.find(orderMessage.orderID) == sell_order_level.end())
        {
            orderMessage.printValues();
            return;
        }

        auto price = sell_order_level[orderMessage.orderID];
        sell_order_book[price].erase_order(order);

        if (sell_order_book[price].empty())
        {
            sell_order_book.erase(price);
        }
        sell_order_level.erase(orderMessage.orderID);
    }

    check_and_insert_active_order();
}

void LevelOrderBook::process_modify_order_message(OrderMessage orderMessage)
{
    // std::cout << "debug: " << std::endl;
    // active_order_message.printValues();

    assert(active_order_message.quantity == 0);
    process_cancel_order_message(orderMessage);
    process_new_order_message(orderMessage);
}

// Trade message
// 1000000000008265
void LevelOrderBook::process_trade_message(TradeMessage trade_message)
{

    // Trade from the ask book
    if (sell_order_level.find(trade_message.sellOrderID) != sell_order_level.end())
    {
        // trade must be happening at price higher than buy price in order book
        assert(trade_message.tradePrice >= sell_order_level[trade_message.sellOrderID]);

        // Trade from order book
        // update order return true if the associated order is finished
        if (sell_order_book[sell_order_level[trade_message.sellOrderID]].update_order(trade_message.sellOrderID, trade_message.tradeQuantity))
        {
            // Also check if that level is empty
            if (sell_order_book[sell_order_level[trade_message.sellOrderID]].empty())
            {
                sell_order_book.erase(sell_order_level[trade_message.sellOrderID]);
            }
            // Delete it from the id to level mapping also
            sell_order_level.erase(trade_message.sellOrderID);
        }
    }
    // Trade from the bid book
    if (buy_order_level.find(trade_message.buyOrderID) != buy_order_level.end())
    {
        // trade must be happening at price lower than buy price in order book
        assert(trade_message.tradePrice <= buy_order_level[trade_message.buyOrderID]);

        // Trade from order book
        // update order return true if the associated order is finished and also delete it from its level
        if (buy_order_book[buy_order_level[trade_message.buyOrderID]].update_order(trade_message.buyOrderID, trade_message.tradeQuantity))
        {
            // Also check if that level is empty
            if (buy_order_book[buy_order_level[trade_message.buyOrderID]].empty())
            {
                buy_order_book.erase(buy_order_level[trade_message.buyOrderID]);
            }
            // Delete it from the id to level mapping also
            buy_order_level.erase(trade_message.buyOrderID);
        }
    }

    if (active_order_message.quantity != 0)
    {
        if (active_order_message.orderID == trade_message.buyOrderID || active_order_message.orderID == trade_message.sellOrderID)
        {
            // Trade the active oreder
            active_order_message.quantity -= trade_message.tradeQuantity;
        }

        check_and_insert_active_order();
    }
}

void LevelOrderBook::print_bid_order_book()
{
    int maxLevels = 8;
    for (auto it = buy_order_book.rbegin(); it != buy_order_book.rend(); ++it)
    {
        maxLevels--;
        std::cout << it->first << ":" << it->second.print_quantity() << " ";
        if (maxLevels == 0)
        {
            break;
        }
    }
}

void LevelOrderBook::print_ask_order_book()
{
    int maxLevels = 8;
    for (auto it = sell_order_book.begin(); it != sell_order_book.end(); ++it)
    {
        maxLevels--;
        std::cout << it->first << ":" << it->second.print_quantity() << " ";
        if (maxLevels == 0)
        {
            break;
        }
    }
}

void LevelOrderBook::print_num_orders()
{
    std::cout << buy_order_book.size() + sell_order_book.size();
}

void LevelOrderBook::print_statistics(long time_stamp)
{
    std::cout << "timestamp: " << time_stamp << std::endl;
    std::cout << "bid_order_book: ";
    print_bid_order_book();
    std::cout << std::endl;
    std::cout << "ask_order_book: ";
    print_ask_order_book();
    std::cout << std::endl;
    std::cout << "orders: ";
    print_num_orders();
    std::cout << std::endl
              << std::endl;
}