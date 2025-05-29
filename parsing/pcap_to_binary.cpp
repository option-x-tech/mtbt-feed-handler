#include <bits/stdc++.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
using namespace std;

typedef struct __attribute__((packed))
{
    uint32_t magic_number;  /* magic number */
    uint16_t version_major; /* major version number */
    uint16_t version_minor; /* minor version number */
    int32_t thiszone;       /* GMT to local correction */
    uint32_t sigfigs;       /* accuracy of timestamps */
    uint32_t snaplen;       /* max length of captured packets, in octets */
    uint32_t network;       /* data link type */
    void print()
    {
        std::cout << "file header:\n";
        std::cout << "    magic_number:" << magic_number << std::endl;
        std::cout << "    version_major:" << version_major << std::endl;
        std::cout << "    version_minor:" << version_minor << std::endl;
        std::cout << "    thiszone:" << thiszone << std::endl;
        std::cout << "    sigfigs:" << sigfigs << std::endl;
        std::cout << "    snaplen:" << snaplen << std::endl;
        std::cout << "    network:" << network << std::endl;
    }
} pcap_hdr_t;

typedef struct __attribute__((packed))
{
    uint32_t ts_sec;   /* timestamp seconds */
    uint32_t ts_usec;  /* timestamp microseconds */
    uint32_t incl_len; /* number of octets of packet saved in file */
    uint32_t orig_len; /* actual length of packet */

    void print()
    {
        std::cout << "pcaprec_hdr_t: \n";
        std::cout << "  timestamp seconds: " << ts_sec << std::endl;
        std::cout << "  timestamp microseconds: " << ts_usec << std::endl;
        std::cout << "  number of octets of packet saved in file: " << incl_len
                  << std::endl;
        std::cout << "  actual length of packet: " << orig_len << std::endl;
    }
} pcaprec_hdr_t;

struct multicast_stream_header
{
    int16_t msg_len;
    int16_t stream_id;
    int32_t seq_num;
    void print();
} __attribute__((packed));

struct order_message
{
    int8_t msg_type; /* Can be N X M or spread orders*/
    int64_t time_stamp;
    double order_id;
    int32_t token;
    int8_t order_type;
    int32_t price;
    int32_t qty;
    void print();
} __attribute__((packed));

struct trade_message
{
    int8_t msg_type; // Can be T
    int64_t time_stamp;
    double_t buy_order_id;
    double_t sell_order_id;
    int32_t token;
    int32_t trade_price;
    int32_t trade_qty;
    void print();
} __attribute__((packed));

struct heartbeat_message
{
    int8_t msg_type; // Can be Z
    int32_t last_seq;
    char pad[5];
} __attribute__((packed));

void multicast_stream_header::print()
{
    std::cout << "      msg_len: " << msg_len << std::endl;
    std::cout << "      stream_id: " << stream_id << std::endl;
    std::cout << "      seq_num: " << seq_num << std::endl;
}

void order_message::print()
{
    std::cout << "      msg_type: " << msg_type << std::endl;
    std::cout << "      time_stamp: " << time_stamp << std::endl;
    std::cout << std::setprecision(20) << "      order_id: " << order_id
              << std::endl;
    std::cout << "      token: " << token << std::endl;
    std::cout << "      order_type: " << order_type << std::endl;
    std::cout << "      price: " << price << std::endl;
    std::cout << "      qty: " << qty << std::endl;
}

void trade_message::print()
{
    std::cout << "      msg_type: " << msg_type << std::endl;
    std::cout << "      time_stamp: " << time_stamp << std::endl;
    std::cout << std::setprecision(20) << "      buy_order_id: " << buy_order_id
              << std::endl;
    std::cout << std::setprecision(20)
              << "      sell_order_id: " << sell_order_id << std::endl;
    std::cout << "      token: " << token << std::endl;
    std::cout << "      trade_price: " << trade_price << std::endl;
    std::cout << "      trade_qty: " << trade_qty << std::endl;
}

std::ifstream read_file(const char *filename)
{
    // Check if the file is opened successfully
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit(1);
    }
    return file;
}

long read_file_size(std::ifstream &file)
{
    // Obtain the file size
    file.seekg(0, std::ios::end);
    long file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    if (file_size == 0)
    {
        std::cerr << "Input file empty" << std::endl;
        exit(1);
    }
    // std::cout << "File Size: " << file_size << std::endl;
    return file_size;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: ./app <path-to-input-pcap>\n";
        exit(1);
    }

    // Take token path from command line input
    const char *filename = argv[1];

    std::ifstream file = read_file(filename);
    long file_size = read_file_size(file);

    long kchunk_size = 100000;
    // Storage buffer for Incremental reading
    std::vector<char>
        buffer(kchunk_size);
    long data_in_buffer = 0;

    // First read the file header of 24 bytes
    pcap_hdr_t *file_header = new pcap_hdr_t;
    file.read(((char *)file_header), sizeof(pcap_hdr_t));
    long index = sizeof(pcap_hdr_t);

    file_header->print();

    // Writing to the binary
    string write_file_name = string(filename) + "_binary.dat";
    auto myfile = std::fstream(write_file_name, std::ios::out | std::ios::binary);

    int last_seq_num = 0;

    // Processing the stream of messages
    while (index < file_size)
    {
        long data_left = file_size - index;
        long free_space_in_buffer = kchunk_size - data_in_buffer;
        long data_to_read = std::min(data_left, free_space_in_buffer);
        file.read((char *)&buffer[0] + data_in_buffer, data_to_read);

        // Process the current buffer array we have
        long buffer_index = 0;
        data_in_buffer += file.gcount();
        while (buffer_index < data_in_buffer)
        {
            // Check if we can read a header
            if ((unsigned)(data_in_buffer - buffer_index) <
                sizeof(pcaprec_hdr_t))
            {
                break;
            }
            pcaprec_hdr_t header;
            std::memcpy(&header, &buffer[buffer_index], sizeof(pcaprec_hdr_t));

            // Check if we can read the message content
            if (data_in_buffer - buffer_index < header.incl_len + sizeof(pcaprec_hdr_t))
            {
                break;
            }

            // header.print();
            // Read data and process it
            index += sizeof(pcaprec_hdr_t);
            buffer_index += sizeof(pcaprec_hdr_t);

            // Read network headers
            ether_header eheader;
            std::memcpy(&eheader, &buffer[buffer_index], sizeof(ether_header));
            index += sizeof(ether_header);
            buffer_index += sizeof(ether_header);

            myfile.write((char *)&eheader, sizeof(ether_header));

            iphdr iheader;
            std::memcpy(&iheader, &buffer[buffer_index], sizeof(iphdr));
            index += sizeof(iphdr);
            buffer_index += sizeof(iphdr);

            myfile.write((char *)&iheader, sizeof(iphdr));

            udphdr uheader;
            std::memcpy(&uheader, &buffer[buffer_index], sizeof(udphdr));
            index += sizeof(udphdr);
            buffer_index += sizeof(udphdr);

            myfile.write((char *)&uheader, sizeof(udphdr));

            // Read the UDP payload
            multicast_stream_header stream_hdr;
            std::memcpy(&stream_hdr, &buffer[buffer_index], sizeof(multicast_stream_header));
            index += sizeof(multicast_stream_header);
            buffer_index += sizeof(multicast_stream_header);
            // Write stream header
            myfile.write((char *)&stream_hdr, sizeof(multicast_stream_header));

            // Read data and process it
            // stream_hdr.print();

            // Packet drop sanity check
            assert(last_seq_num == 0 || last_seq_num + 1 == stream_hdr.seq_num);
            last_seq_num = stream_hdr.seq_num;

            char msg_type = buffer[buffer_index];
            if (msg_type == 'T')
            {
                trade_message message;
                std::memcpy(&message, &buffer[buffer_index], sizeof(trade_message));
                index += sizeof(trade_message);
                buffer_index += sizeof(trade_message);
                // message.print();

                // Write trade message
                myfile.write((char *)&message, sizeof(trade_message));
            }
            else if (msg_type == 'K' || msg_type == 'C')
            {
                trade_message message;
                std::memcpy(&message, &buffer[buffer_index], sizeof(trade_message));
                index += sizeof(trade_message);
                buffer_index += sizeof(trade_message);
                // message.print();

                // Write trade message
                myfile.write((char *)&message, sizeof(trade_message));
            }
            else if (msg_type == 'Z')
            {
                heartbeat_message message;
                std::memcpy(&message, &buffer[buffer_index], sizeof(heartbeat_message));
                index += sizeof(heartbeat_message);
                buffer_index += sizeof(heartbeat_message);

                // Write order message
                myfile.write((char *)&message, sizeof(heartbeat_message));
            }
            else if (msg_type == 'N' || msg_type == 'M' || msg_type == 'X')
            {
                order_message message;
                std::memcpy(&message, &buffer[buffer_index], sizeof(order_message));
                index += sizeof(order_message);
                buffer_index += sizeof(order_message);
                // message.print();

                // Write order message
                myfile.write((char *)&message, sizeof(order_message));
            }
            else if (msg_type == 'G' || msg_type == 'H' || msg_type == 'J')
            {
                order_message message;
                std::memcpy(&message, &buffer[buffer_index], sizeof(order_message));
                index += sizeof(order_message);
                buffer_index += sizeof(order_message);
                // message.print();

                // Write order message
                myfile.write((char *)&message, sizeof(order_message));
            }
            else
            {
                cout << "Assert Error" << endl;
                cout << msg_type << endl;
                exit(0);
            }
            // cout << endl;
        }

        // Some of the buffer array if not processed, move it to the start of it
        std::memmove(&buffer[0], &buffer[buffer_index],
                     data_in_buffer - buffer_index);
        data_in_buffer = data_in_buffer - buffer_index;
    }
    // End of file parsing
    myfile.close();
}