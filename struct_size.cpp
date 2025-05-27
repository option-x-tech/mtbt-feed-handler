#include <bits/stdc++.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
using namespace std;

struct multicast_stream_header
{
    int16_t msg_len;
    int16_t stream_id;
    int32_t seq_num;
    void print(); 
} __attribute__((packed));

struct order_message {
    int8_t  msg_type; /* Can be N X M or spread orders*/
    int64_t time_stamp;
    double  order_id;
    int32_t token;
    int8_t  order_type;
    int32_t price;
    int32_t qty;
    void    print();
} __attribute__( ( packed ) );
  
struct trade_message {
    int8_t   msg_type; // Can be T
    int64_t  time_stamp;
    double_t buy_order_id;
    double_t sell_order_id;
    int32_t  token;
    int32_t  trade_price;
    int32_t  trade_qty;
    void     print();
} __attribute__( ( packed ) );

struct heartbeat_message {
    int8_t   msg_type; // Can be Z
    int32_t last_seq;  
    char pad[5]; 
} __attribute__( ( packed ) );



int main(int argc, char **argv)
{
    cout << "ether_header size: " << sizeof(ether_header) << endl; 
    cout << "iphdr size: " << sizeof(iphdr) << endl; 
    cout << "udphdr size: " << sizeof(udphdr) << endl; 

    cout << "multicast stream header size: " << sizeof(multicast_stream_header) << endl; 
    cout << "order_message size: " << sizeof(order_message) << endl; 
    cout << "trade_message size: " << sizeof(trade_message) << endl; 
    cout << "heartbeat_message size: " << sizeof(heartbeat_message) << endl; 
}