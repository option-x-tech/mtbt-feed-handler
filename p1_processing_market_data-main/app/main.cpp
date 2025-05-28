#include "read_data.hpp"
#include "order_book.hpp"
#include "level_order_book.hpp"

int main()
{
    const char *filename = "/home/mradul/mtbt-data-handling/data/28May/bondteam3_trimmed.pcap_binary.dat";
    std::vector<unsigned char> fileData = readToken(filename);

    int index = 0;
    int fileSize = fileData.size();

    std::cout << "File Size: " << fileSize << std::endl;

    // OrderBook order_book;
    LevelOrderBook order_book;

    long timer = -1;

    // Processing the stream of messages
    while (index < fileSize)
    {
        StreamHeader header;
        std::memcpy(&header, &fileData[index], sizeof(header));
        // header.printValues();
        index += sizeof(StreamHeader);
        int dataLength = header.messageLen - sizeof(StreamHeader);

        char message_type = fileData[index];
        assert(dataLength == sizeof(OrderMessage) || dataLength == sizeof(TradeMessage));

        if (dataLength == sizeof(OrderMessage))
        {
            OrderMessage orderMessage;
            std::memcpy(&orderMessage, &fileData[index], sizeof(OrderMessage));
            index += sizeof(OrderMessage);
            // Before processing print statistics
            long timestamp = orderMessage.timestamp;
            if (timestamp >= timer + 10000000000 || timer == -1)
            {
                timer = timestamp + 10000000000;
                order_book.print_statistics(timestamp);
            }
            if (orderMessage.token == 61781)
            {
                // orderMessage.printValues();
                if (message_type == 'N')
                {
                    order_book.process_new_order_message(orderMessage);
                }
                else if (message_type == 'X')
                {
                    order_book.process_cancel_order_message(orderMessage);
                }
                else if (message_type == 'M')
                {
                    order_book.process_modify_order_message(orderMessage);
                }
                // order_book.print_statistics(timestamp);
            }

        }
        else if (dataLength == sizeof(TradeMessage))
        {
            TradeMessage tradeMessage;
            std::memcpy(&tradeMessage, &fileData[index], sizeof(TradeMessage));
            index += sizeof(TradeMessage);
            // Before processing print statistics
            long timestamp = tradeMessage.timestamp;
            if (timestamp >= timer + 10000000000 || timer == -1)
            {
                timer = timestamp + 10000000000;
                order_book.print_statistics(timestamp);
            }
            if (tradeMessage.token == 61781)
            {
                // tradeMessage.printValues();
                order_book.process_trade_message(tradeMessage);
                // order_book.print_statistics(timestamp);
            }
        }
    }
}