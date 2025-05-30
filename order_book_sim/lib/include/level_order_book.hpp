#ifndef P1_LEVEL_OrderBook_
#define P1_LEVEL_OrderBook_

#include "read_data.hpp"

class Order
{
public:
    long timestamp;
    int price;
    int quantity;
    Order(long timestamp_, int price_, int quantity_) : timestamp(timestamp_), price(price_), quantity(quantity_) {}
    Order(const Order &order) : timestamp(order.timestamp), price(order.price), quantity(order.quantity) {}
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

template <typename T, size_t BlockSize = 4096>
class MemoryPool
{
private:
    union alignas(T) Chunk
    { // Ensure proper alignment for T
        T object;
        Chunk *next;
    };

    Chunk *freeList = nullptr;
    std::vector<void *> blocks;

public:
    MemoryPool()
    {
        static_assert(BlockSize >= sizeof(Chunk),
                      "BlockSize must be at least sizeof(Chunk)");
        allocateBlock();
    }

    T *allocate()
    {
        if (!freeList)
            allocateBlock();

        Chunk *chunk = freeList;
        freeList = freeList->next;
        return &chunk->object; // Properly aligned T*
    }

    void deallocate(T *ptr) noexcept
    {
        Chunk *chunk = reinterpret_cast<Chunk *>(ptr);
        chunk->next = freeList;
        freeList = chunk;
    }

private:
    void allocateBlock()
    {
        // Allocate aligned memory for the block
        void *block;
        const size_t alignment = alignof(Chunk);
        if (posix_memalign(&block, alignment, BlockSize) != 0)
        {
            throw std::bad_alloc();
        }

        blocks.push_back(block);
        const size_t chunkCount = BlockSize / sizeof(Chunk);

        // Initialize free list links
        Chunk *chunk = static_cast<Chunk *>(block);
        freeList = chunk;

        for (size_t i = 0; i < chunkCount - 1; ++i)
        {
            chunk->next = chunk + 1;
            chunk = chunk->next;
        }
        chunk->next = nullptr;
    }
};

class LevelOrderBook
{
    // Vector of Levels sorted on price
    std::vector<Level> sell_order_book; // Low to High
    std::vector<Level> buy_order_book;  // High to Low

    // Unordered map of orderId to its order metadata
    std::unordered_map<double, Order *> buy_order_level;
    std::unordered_map<double, Order *> sell_order_level;

    int get_best_bid();
    int get_best_ask();

    OrderMessage active_order_message;

    MemoryPool<Order> orderPool;

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