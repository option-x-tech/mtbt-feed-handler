#include "level_order_book.hpp"

int LevelOrderBook::get_best_bid()
{
    if (!buy_order_book.empty())
        return buy_order_book.begin()->price;
    return INT32_MIN;
}

int LevelOrderBook::get_best_ask()
{
    if (!sell_order_book.empty())
        return sell_order_book.begin()->price;
    return INT32_MAX;
}

void LevelOrderBook::process_new_order_message(OrderMessage &orderMessage)
{

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
        if (get_best_ask() <= orderMessage.price)
        {
            active_order_message = orderMessage;
            return;
        }
    }
    if (orderMessage.orderType == 'S')
    {
        if (get_best_bid() >= orderMessage.price)
        {
            active_order_message = orderMessage;
            return;
        }
    }

    if (orderMessage.orderType == 'B')
    {
        // Confirm that there is not a order already present with this ID
        assert(buy_order_level.find(orderMessage.orderID) == buy_order_level.end());
        auto buy_order_book_iterator = buy_order_book.begin();
        bool level_found = false;
        while (buy_order_book_iterator != buy_order_book.end())
        {
            if (buy_order_book_iterator->price == orderMessage.price)
            {
                buy_order_book_iterator->total_quantity += orderMessage.quantity;
                Order order(orderMessage.timestamp, orderMessage.price, orderMessage.quantity, buy_order_book_iterator);
                buy_order_level[orderMessage.orderID] = order;
                level_found = true;
                break;
            }
            else if (buy_order_book_iterator->price < orderMessage.price)
            {
                buy_order_book_iterator = buy_order_book.insert(buy_order_book_iterator, Level(orderMessage.quantity, orderMessage.price));
                Order order(orderMessage.timestamp, orderMessage.price, orderMessage.quantity, buy_order_book_iterator);
                buy_order_level[orderMessage.orderID] = order;
                level_found = true;
                break;
            }
            ++buy_order_book_iterator;
        }
        if (!level_found)
        {
            buy_order_book_iterator = buy_order_book.insert(buy_order_book_iterator, Level(orderMessage.quantity, orderMessage.price));
            Order order(orderMessage.timestamp, orderMessage.price, orderMessage.quantity, buy_order_book_iterator);
            buy_order_level[orderMessage.orderID] = order;
        }
    }

    if (orderMessage.orderType == 'S')
    {
        // Confirm that there is not a order already present with this ID
        assert(sell_order_level.find(orderMessage.orderID) == sell_order_level.end());
        auto sell_order_book_iterator = sell_order_book.begin();
        bool level_found = false;
        while (sell_order_book_iterator != sell_order_book.end())
        {
            if (sell_order_book_iterator->price == orderMessage.price)
            {
                sell_order_book_iterator->total_quantity += orderMessage.quantity;
                Order order(orderMessage.timestamp, orderMessage.price, orderMessage.quantity, sell_order_book_iterator);
                sell_order_level[orderMessage.orderID] = order;
                level_found = true;
                break;
            }
            else if (sell_order_book_iterator->price > orderMessage.price)
            {
                sell_order_book_iterator = sell_order_book.insert(sell_order_book_iterator, Level(orderMessage.quantity, orderMessage.price));
                Order order(orderMessage.timestamp, orderMessage.price, orderMessage.quantity, sell_order_book_iterator);
                sell_order_level[orderMessage.orderID] = order;
                level_found = true;
                break;
            }
            ++sell_order_book_iterator;
        }
        if (!level_found)
        {
            sell_order_book_iterator = sell_order_book.insert(sell_order_book_iterator, Level(orderMessage.quantity, orderMessage.price));
            Order order(orderMessage.timestamp, orderMessage.price, orderMessage.quantity, sell_order_book_iterator);
            sell_order_level[orderMessage.orderID] = order;
        }
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

void LevelOrderBook::process_cancel_order_message(OrderMessage &orderMessage)
{

    // If both the Buy and Ask orderbooks are not empty then check if they are not crossing
    if (!buy_order_book.empty() && !sell_order_book.empty())
    {
        assert(get_best_bid() < get_best_ask());
    }

    // If an active order is present
    if (active_order_message.quantity != 0)
    {
        // Check if the cancel message is for the active order
        if (orderMessage.orderID == active_order_message.orderID)
        {
            active_order_message.quantity = 0;
            return;
        }
    }

    // If an active order is not present or the cancel message is not for it
    if (orderMessage.orderType == 'B')
    {
        auto buy_order_iterator = buy_order_level.find(orderMessage.orderID);

        // sometimes there is no such order id for which this cancellation or modification is done
        if (buy_order_iterator == buy_order_level.end())
        {
            // orderMessage.printValues();
            return;
        }

        auto buy_order_book_iterator = buy_order_iterator->second.order_iterator; 

        // Update the book
        buy_order_book_iterator->total_quantity -= buy_order_iterator->second.quantity;
        assert(buy_order_book_iterator->total_quantity >= 0);
        // Check for empty Level
        if (buy_order_book_iterator->total_quantity == 0)
        {
            buy_order_book.erase(buy_order_book_iterator);
        }
        // Update metadata
        buy_order_level.erase(buy_order_iterator);
    }
    if (orderMessage.orderType == 'S')
    {
        auto sell_order_iterator = sell_order_level.find(orderMessage.orderID);

        // sometimes there is no such order id for which this cancellation or modification is done
        if (sell_order_iterator == sell_order_level.end())
        {
            // orderMessage.printValues();
            return;
        }

        auto sell_order_book_iterator = sell_order_iterator->second.order_iterator;

        // Update the book
        sell_order_book_iterator->total_quantity -= sell_order_iterator->second.quantity;
        assert(sell_order_book_iterator->total_quantity >= 0);
        // Check for empty Level
        if (sell_order_book_iterator->total_quantity == 0)
        {
            sell_order_book.erase(sell_order_book_iterator);
        }
        // Update metadata
        sell_order_level.erase(sell_order_iterator);
    }

    check_and_insert_active_order();
}

void LevelOrderBook::process_modify_order_message(OrderMessage &orderMessage)
{
    // std::cout << "debug: " << std::endl;
    // active_order_message.printValues();

    assert(active_order_message.quantity == 0);
    process_cancel_order_message(orderMessage);
    process_new_order_message(orderMessage);
}

// Trade message
void LevelOrderBook::process_trade_message(TradeMessage &trade_message)
{

    // Trade from the ask book
    if (sell_order_level.find(trade_message.sellOrderID) != sell_order_level.end())
    {
        auto &ref_order = sell_order_level[trade_message.sellOrderID];

        // trade must be happening at price higher than buy price in order book
        assert(trade_message.tradePrice >= ref_order.price);

        // Trade the order
        ref_order.quantity -= trade_message.tradeQuantity;
        auto sell_order_book_iterator = sell_order_book.begin();
        while (sell_order_book_iterator != sell_order_book.end())
        {
            if (ref_order.price == sell_order_book_iterator->price)
            {
                // Update the book
                sell_order_book_iterator->total_quantity -= trade_message.tradeQuantity;
                assert(sell_order_book_iterator->total_quantity >= 0);
                // Check for empty Level
                if (sell_order_book_iterator->total_quantity == 0)
                {
                    sell_order_book.erase(sell_order_book_iterator);
                }
                break;
            }
            ++sell_order_book_iterator;
        }
        // Update metadata
        if (ref_order.quantity == 0)
        {
            sell_order_level.erase(trade_message.sellOrderID);
        }
    }
    // Trade from the bid book
    if (buy_order_level.find(trade_message.buyOrderID) != buy_order_level.end())
    {
        auto &ref_order = buy_order_level[trade_message.buyOrderID];

        // trade must be happening at price lower than sell price in order book
        assert(trade_message.tradePrice <= ref_order.price);

        // Trade the order
        ref_order.quantity -= trade_message.tradeQuantity;
        auto buy_order_book_iterator = buy_order_book.begin();
        while (buy_order_book_iterator != buy_order_book.end())
        {
            if (ref_order.price == buy_order_book_iterator->price)
            {
                // Update the book
                buy_order_book_iterator->total_quantity -= trade_message.tradeQuantity;
                assert(buy_order_book_iterator->total_quantity >= 0);
                // Check for empty Level
                if (buy_order_book_iterator->total_quantity == 0)
                {
                    buy_order_book.erase(buy_order_book_iterator);
                }
                break;
            }
            ++buy_order_book_iterator;
        }
        // Update metadata
        if (ref_order.quantity == 0)
        {
            buy_order_level.erase(trade_message.buyOrderID);
        }
    }

    if (active_order_message.quantity != 0)
    {
        if (active_order_message.orderID == trade_message.buyOrderID || active_order_message.orderID == trade_message.sellOrderID)
        {
            // Trade the active oreder
            active_order_message.quantity -= trade_message.tradeQuantity;
            assert(active_order_message.quantity >= 0);
        }

        check_and_insert_active_order();
    }
}

void LevelOrderBook::print_bid_order_book()
{
    int maxLevels = 8;
    for (auto it = buy_order_book.begin(); it != buy_order_book.end(); ++it)
    {
        maxLevels--;
        std::cout << it->price << ":" << it->total_quantity << " ";
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
        std::cout << it->price << ":" << it->total_quantity << " ";
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

void LevelOrderBook::print_statistics(int token, long time_stamp)
{
    std::cout << "token: " << token << std::endl;
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