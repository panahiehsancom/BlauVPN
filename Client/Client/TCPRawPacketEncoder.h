#pragma once
#pragma warning( disable : 4996)

#include <memory>
#include <string>
#include <vector> 

#include <WinSock2.h>


class TCPRawPacketEncoder
{
public:

	 

	struct tcp_pseudo /*the tcp pseudo header*/
	{
		unsigned int src_addr;
		unsigned int dst_addr;
		unsigned char zero;
		unsigned char proto;
		unsigned short length;
	} pseudohead;


	struct iphdr
	{
		unsigned char ihl : 4;
		unsigned char version : 4;
		uint8_t tos;
		uint16_t tot_len;
		uint16_t id;
		uint16_t frag_off;
		uint8_t ttl;
		uint8_t protocol;
		uint16_t check;
		uint32_t saddr;
		uint32_t daddr;
		/*The options start here. */
	};
	struct tcphdr
	{
		uint16_t source;
		uint16_t dest;
		uint32_t seq;
		uint32_t ack_seq;
		unsigned char res1 : 4;
		unsigned char doff : 4;
		unsigned char fin : 1;
		unsigned char syn : 1;
		unsigned char rst : 1;
		unsigned char psh : 1;
		unsigned char ack : 1;
		unsigned char urg : 1;
		unsigned char res2 : 2;
		uint16_t window;
		unsigned short check;
		uint16_t urg_ptr;
	};
	  

	TCPRawPacketEncoder::iphdr get_ipheader(std::vector<char> buffer);
	TCPRawPacketEncoder::tcphdr get_tcp_header(std::vector<char> buffer);
	std::string get_source_ipaddress(std::vector<char> buffer);
	std::string get_destination_ipaddress(std::vector<char> buffer);
	unsigned short get_source_port(std::vector<char> buffer);
	unsigned short get_destination_port(std::vector<char> buffer);
	
};

