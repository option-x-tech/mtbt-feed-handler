#include "read_data.hpp"
#include "level_order_book.hpp"

int main()
{
    auto start_time = std::chrono::high_resolution_clock::now();

    const char *filename = "/home/mradul/mtbt-data-handling/data/28May/bondteam3_trimmed.pcap_binary.dat";
    std::vector<unsigned char> fileData = readToken(filename);

    int index = 0;
    int fileSize = fileData.size();

    std::cout << "File Size: " << fileSize << std::endl;

    // Many order books
    // std::unordered_map<int, LevelOrderBook> order_books;
    std::vector<LevelOrderBook> order_books;
    std::unordered_map<int, int> token_index_mapping;

    int current_index = 0;

    long timer = -1;

    int low = INT32_MAX;
    int high = 0;

    // Processing the stream of messages
    while (index < fileSize)
    {

        // Skip the network headers
        index += sizeof(ether_header);
        index += sizeof(iphdr);
        index += sizeof(udphdr);

        StreamHeader header;
        std::memcpy(&header, &fileData[index], sizeof(header));
        // header.printValues();
        index += sizeof(StreamHeader);
        int dataLength = header.messageLen - sizeof(StreamHeader);

        char message_type = fileData[index];

        if (message_type == 'N' || message_type == 'M' || message_type == 'X')
        {
            OrderMessage orderMessage;
            std::memcpy(&orderMessage, &fileData[index], sizeof(OrderMessage));
            index += sizeof(OrderMessage);
            // Before processing print statistics
            long timestamp = orderMessage.timestamp;

            int token = orderMessage.token;
            auto token_index_iterator = token_index_mapping.find(token);
            int index; 
            if (token_index_iterator == token_index_mapping.end())
            {
                order_books.push_back(LevelOrderBook()); 
                index = current_index; 
                token_index_mapping[token] = current_index++;
            }
            else 
            {
                index = token_index_iterator->second; 
            }
            LevelOrderBook &order_book = order_books[index];

            low = std::min(low, token);
            high = std::max(high, token);

            if (timestamp >= timer + 10000000000 || timer == -1)
            {
                timer = timestamp + 10000000000;
                order_book.print_statistics(token, timestamp);
            }

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
        }
        else if (message_type == 'T')
        {
            TradeMessage tradeMessage;
            std::memcpy(&tradeMessage, &fileData[index], sizeof(TradeMessage));
            index += sizeof(TradeMessage);
            // Before processing print statistics
            long timestamp = tradeMessage.timestamp;

            int token = tradeMessage.token;
            auto token_index_iterator = token_index_mapping.find(token);
            if (token_index_iterator == token_index_mapping.end())
            {
                order_books.push_back(LevelOrderBook());
                token_index_mapping[token] = current_index++;
            }
            LevelOrderBook &order_book = order_books[token_index_mapping[token]];

            if (timestamp >= timer + 10000000000 || timer == -1)
            {
                timer = timestamp + 10000000000;
            }

            // tradeMessage.printValues();
            // order_book.print_statistics(token, timestamp);
            order_book.process_trade_message(tradeMessage);
        }
        else if (message_type == 'K' || message_type == 'C')
        {
            index += sizeof(TradeMessage);
        }
        else if (message_type == 'Z')
        {
            index += sizeof(heartbeat_message);
        }
        else if (message_type == 'G' || message_type == 'H' || message_type == 'J')
        {
            index += sizeof(OrderMessage);
        }
        else
        {
            std::cout << message_type << std::endl;
            exit(0);
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double time_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    std::cout << "Elapsed time: " << time_elapsed  << " Milliseconds" << std::endl;
    std::cout << "Data rate: " << fileSize / time_elapsed << "MBps" << std::endl;

    std::cout << low << " " << high << std::endl;
}