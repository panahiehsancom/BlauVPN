// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#pragma warning( disable : 4996)

#include <cstdio>
#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "TCPClient.h"

#include <WinSock2.h>
std::vector<std::shared_ptr<TCPClient>> clients;
std::shared_ptr<TCPClient> server_communication_;


typedef struct ip_hdr
{
	unsigned char ip_header_len : 4; // 4-bit header length (in 32-bit words) normally=5 (Means 20 Bytes may be 24 also)
	unsigned char ip_version : 4; // 4-bit IPv4 version
	unsigned char ip_tos; // IP type of service
	unsigned short ip_total_length; // Total length
	unsigned short ip_id; // Unique identifier

	unsigned char ip_frag_offset : 5; // Fragment offset field

	unsigned char ip_more_fragment : 1;
	unsigned char ip_dont_fragment : 1;
	unsigned char ip_reserved_zero : 1;

	unsigned char ip_frag_offset1; //fragment offset

	unsigned char ip_ttl; // Time to live
	unsigned char ip_protocol; // Protocol(TCP,UDP etc)
	unsigned short ip_checksum; // IP checksum
	unsigned int ip_srcaddr; // Source address
	unsigned int ip_destaddr; // Destination address
} IPV4_HDR, * PIPV4_HDR, FAR* LPIPV4_HDR;

// TCP header
typedef struct tcp_header
{
	unsigned short source_port; // source port
	unsigned short dest_port; // destination port
	unsigned int sequence; // sequence number - 32 bits
	unsigned int acknowledge; // acknowledgement number - 32 bits

	unsigned char ns : 1; //Nonce Sum Flag Added in RFC 3540.
	unsigned char reserved_part1 : 3; //according to rfc
	unsigned char data_offset : 4; /*The number of 32-bit words in the TCP header.
	This indicates where the data begins.
	The length of the TCP header is always a multiple
	of 32 bits.*/

	unsigned char fin : 1; //Finish Flag
	unsigned char syn : 1; //Synchronise Flag
	unsigned char rst : 1; //Reset Flag
	unsigned char psh : 1; //Push Flag
	unsigned char ack : 1; //Acknowledgement Flag
	unsigned char urg : 1; //Urgent Flag

	unsigned char ecn : 1; //ECN-Echo Flag
	unsigned char cwr : 1; //Congestion Window Reduced Flag

	////////////////////////////////

	unsigned short window; // window
	unsigned short checksum; // checksum
	unsigned short urgent_pointer; // urgent pointer
} TCP_HDR, * PTCP_HDR, FAR* LPTCP_HDR, TCPHeader, TCP_HEADER;

void client_data_received(std::string id, const char* data, size_t size)
{

}
std::string get_hex(char value)
{
	std::stringstream sstream;
	sstream << std::hex << static_cast<int>(value);
	std::string result = sstream.str();
	for (int i = result.size(); i < 2; i++)
		result = "0" + result;
	result  = result.substr(0, 2);
	return result;
}

void print_hex(std::vector<char> buffer)
{
	std::string value = "";
	for (char c: buffer)
	{
		value += get_hex(c) + "-";
	}
	printf(value.c_str());
	printf("\n");
}

void data_received(std::string id, const char* data, size_t size)
{
	std::string raw_str(data, size);
	printf("Raw Input String  is : \n%s", data);
	printf("Raw Hex is \n");
	std::vector<char> input_buffer(data, data + size);
	print_hex(input_buffer);
	ip_hdr * iph = (ip_hdr*) data;
	tcp_header * tcph = (tcp_header*) (data + sizeof(iph));

	struct in_addr src_ip_addr;
	src_ip_addr.s_addr = (iph->ip_srcaddr);
	const char* sourceip = inet_ntoa(src_ip_addr);

	printf("source ip address is : %s\n", sourceip);

	struct in_addr dest_ip_addr;
	dest_ip_addr.s_addr = (iph->ip_destaddr);
	const char* dest_ip = inet_ntoa(dest_ip_addr);

	printf("Destination ip address is : %s\n", dest_ip);

	////std::string payload(data + sizeof(iph) + sizeof(iph), data + size);
	//std::string destination_ipaddress;
	//std::string port_number;
	//std::string protocol;
	//
	//if (protocol == "tcp")
	//{
	//	std::shared_ptr<TCPClient> client = std::make_shared<TCPClient>(destination_ipaddress + ":"+ port_number);
	//	client->connect_on_data_received(std::bind(client_data_received, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	//	client->connect(destination_ipaddress, port_number);
	//}

}
int wmain(int argc, wchar_t * argv[])
{
	if (argc >= 3)
	{
		std::wstring server_address;
		std::wstring port_number;
		int index = 0;
		while (index < argc)
		{
			std::wstring value = argv[index];
			if (value.compare(L"-s") == 0)
			{
				server_address = argv[++index];
			}
			if (value.compare(L"-p") == 0)
			{
				port_number = argv[++index];
			}
			index++;
		}
		std::string str_ip(server_address.begin(), server_address.end());
		std::string str_port(port_number.begin(), port_number.end());
		server_communication_ = std::make_shared<TCPClient>("Server");
		server_communication_->connect_on_data_received(std::bind(data_received, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		server_communication_->connect(str_ip, str_port);
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::seconds(10));
		}

	}
	else
	{
		printf("invalid arguments");
		printf("-s server address -p port number");
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
