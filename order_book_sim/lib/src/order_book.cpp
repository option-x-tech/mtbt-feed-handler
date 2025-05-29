#include "order_book.hpp"

int OrderBook::get_best_bid()
{
    if (!buyOrderBook.empty())
        return buyOrderBook.rbegin()->price;
    return INT32_MIN;
}
int OrderBook::get_best_ask()
{
    if (!sellOrderBook.empty())
        return sellOrderBook.begin()->price;
    return INT32_MAX;
}

void OrderBook::process_order_message(OrderMessage orderMessage)
{
    Order order(orderMessage.timestamp, orderMessage.orderID, orderMessage.price, orderMessage.quantity);
    // If both the Buy and Ask orderbooks are not empty then check if they are not crossing
    if (!buyOrders.empty() && !sellOrders.empty())
    {
        assert(get_best_bid() < get_best_ask());
    }

    // New or a Mofication message arrives (consider it as an active order)
    if (orderMessage.messageType == 'N' || orderMessage.messageType == 'M')
    {
        if (orderMessage.orderType == 'B')
        {
            if (buyOrders.find(orderMessage.orderID) == buyOrders.end())
            {
                buyOrders[orderMessage.orderID] = order;
                buyOrderBook.insert(order);
            }
            else
            {
                buyOrderBook.erase(buyOrders[orderMessage.orderID]);
                buyOrders[orderMessage.orderID] = order;
                buyOrderBook.insert(order);
            }
        }
        if (orderMessage.orderType == 'S')
        {
            if (sellOrders.find(orderMessage.orderID) == sellOrders.end())
            {
                sellOrders[orderMessage.orderID] = order;
                sellOrderBook.insert(order);
            }
            else
            {
                sellOrderBook.erase(sellOrders[orderMessage.orderID]);
                sellOrders[orderMessage.orderID] = order;
                sellOrderBook.insert(order);
            }
        }
    }
    // Cancellation order arrives
    else if (orderMessage.messageType == 'X')
    {
        if (orderMessage.orderType == 'B')
        {
            assert(buyOrders.find(orderMessage.orderID) != buyOrders.end());
            buyOrderBook.erase(buyOrders[orderMessage.orderID]);
            buyOrders.erase(orderMessage.orderID);
        }
        if (orderMessage.orderType == 'S')
        {
            assert(sellOrders.find(orderMessage.orderID) != sellOrders.end());
            sellOrderBook.erase(sellOrders[orderMessage.orderID]);
            sellOrders.erase(orderMessage.orderID);
        }
    }
}

void OrderBook::process_trade_message(TradeMessage tradeMessage)
{
    if (buyOrders.find(tradeMessage.buyOrderID) != buyOrders.end())
    {
        assert(tradeMessage.tradePrice <= buyOrders[tradeMessage.buyOrderID].price);

        buyOrderBook.erase(buyOrders[tradeMessage.buyOrderID]);
        buyOrders[tradeMessage.buyOrderID].quantity -= tradeMessage.tradeQuantity;
      
        assert(buyOrders[tradeMessage.buyOrderID].quantity >= 0);
        if (buyOrders[tradeMessage.buyOrderID].quantity == 0)
        {
            buyOrders.erase(tradeMessage.buyOrderID);
        }
        else
        {
            buyOrderBook.insert(buyOrders[tradeMessage.buyOrderID]);
        }
    }
    if (sellOrders.find(tradeMessage.sellOrderID) != sellOrders.end())
    {
        assert(tradeMessage.tradePrice >= sellOrders[tradeMessage.sellOrderID].price);

        sellOrderBook.erase(sellOrders[tradeMessage.sellOrderID]);
        sellOrders[tradeMessage.sellOrderID].quantity -= tradeMessage.tradeQuantity;

        assert(sellOrders[tradeMessage.sellOrderID].quantity >= 0);
        if (sellOrders[tradeMessage.sellOrderID].quantity == 0)
        {
            sellOrders.erase(tradeMessage.sellOrderID);
        }
        else
        {
            sellOrderBook.insert(sellOrders[tradeMessage.sellOrderID]);
        }
    }
}

void OrderBook::print_bid_order_book()
{
    int maxLevels = 8; 
    int last_level = -1; 
    int sum = 0;
    for(auto it = buyOrderBook.rbegin(); it != buyOrderBook.rend(); ++it)
    {
        if(last_level == -1) 
        {
            last_level = it->price; 
        }
        if(it->price != last_level) 
        {
            maxLevels--;
            std::cout << last_level << ":" << sum << " "; 
            sum = 0;
            last_level = it->price; 
            if(maxLevels == 0)
            {
                break; 
            }
        }
        sum += it->quantity; 
    }
    if(maxLevels > 0 && sum > 0 && last_level != -1) 
    {
        std::cout << last_level << ":" << sum << " "; 
    }
}

void OrderBook::print_ask_order_book()
{
    int maxLevels = 8; 
    int last_level = -1; 
    int sum = 0;
    for(auto it = sellOrderBook.begin(); it != sellOrderBook.end(); ++it)
    {
        if(last_level == -1) 
        {
            last_level = it->price; 
        }
        if(it->price != last_level) 
        {
            maxLevels--;
            std::cout << last_level << ":" << sum << " "; 
            sum = 0;
            last_level = it->price; 
            if(maxLevels == 0)
            {
                break; 
            }
        }
        sum += it->quantity; 
    }
    if(maxLevels > 0 && sum > 0 && last_level != -1) 
    {
        std::cout << last_level << ":" << sum << " "; 
    }
}

void OrderBook::print_num_orders()
{
    std::cout << buyOrders.size() + sellOrders.size(); 
}

void OrderBook::print_statistics(long time_stamp)
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
    std::cout << std::endl << std::endl; 
}