#ifndef P1_READ_DATA_
#define P1_READ_DATA_

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <assert.h>
#include <set>
#include <map>

struct StreamHeader
{
    short messageLen;
    short streamID;
    int sequenceNumber;
    void printValues();
};

#pragma pack(push, 1)
struct OrderMessage
{
    char messageType;
    long timestamp;
    double orderID;
    int token;
    char orderType;
    int price;
    int quantity;
    void printValues();
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TradeMessage
{
    char messageType;
    long timestamp;
    double buyOrderID;
    double sellOrderID;
    int token;
    int tradePrice;
    int tradeQuantity;
    void printValues();
};
#pragma pack(pop)

std::vector<unsigned char> readToken(const char *filename);

#endif // P1_READ_DATA_