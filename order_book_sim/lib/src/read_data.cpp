#include "read_data.hpp"

void StreamHeader::printValues()
{
    std::cout << "Header Attributes" << std::endl;
    std::cout << "  Message Length: " << messageLen << std::endl;
    std::cout << "  Stream ID: " << streamID << std::endl;
    std::cout << "  Sequence Number: " << sequenceNumber << std::endl;
}

void OrderMessage::printValues()
{
    std::cout << "Order Message Attributes" << std::endl;
    std::cout << "  Message Type: " << messageType << std::endl;
    std::cout << "  Time Stamp: " << timestamp << std::endl;
    std::cout << std::setprecision(30) << "  Order ID: " << orderID << std::endl;
    std::cout << "  Token: " << token << std::endl;
    std::cout << "  Order Type: " << orderType << std::endl;
    std::cout << "  Price: " << price << std::endl;
    std::cout << "  Quantity: " << quantity << std::endl;
}

void TradeMessage::printValues()
{
    std::cout << "Trade Message Attributes" << std::endl;
    std::cout << "  Message Type: " << messageType << std::endl;
    std::cout << "  Time Stamp: " << timestamp << std::endl;
    std::cout << std::setprecision(30) << "  Buy Order ID: " << buyOrderID << std::endl;
    std::cout << std::setprecision(30) << "  Sell Order ID: " << sellOrderID << std::endl;
    std::cout << "  Token: " << token << std::endl;
    std::cout << "  Trade Price: " << tradePrice << std::endl;
    std::cout << "  Trade Quantity: " << tradeQuantity << std::endl;
}

std::vector<unsigned char> readToken(const char *filename)
{
    std::streampos fileSize;
    std::ifstream file(filename, std::ios::binary);

    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> fileData(fileSize);
    file.read((char *)&fileData[0], fileSize);

    return fileData;
}