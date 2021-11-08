#pragma once
#pragma warning( disable : 4996)

#include <vector> 
#include <string>


#include <WinSock2.h>


class TCPRawPacketEncoder
{
public:

	struct iphdr {
		unsigned char	ihl : 4,
			unsigned char	version : 4;
		unsigned char	tos;
		unsigned short	tot_len;
		unsigned short	id;
		unsigned short	frag_off;
		unsigned char	ttl;
		unsigned char	protocol;
		unsigned short	check;
		unsigned int	saddr;
		unsigned int	daddr;
		/*The options start here. */
	};

	struct pseudo_tcp_header
	{
		struct in_addr source, dest;
		u_char reserved;
		u_char protocol;
		u_short tcp_size;
	};

	typedef struct IPHeader
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
	};

	// TCP header
	typedef struct TCPHeader
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
		 
	};
	  

	IPHeader get_ipheader(std::vector<char> buffer);
	TCPHeader get_tcp_header(std::vector<char> buffer);
	std::string get_source_ipaddress(std::vector<char> buffer);
	std::string get_destination_ipaddress(std::vector<char> buffer);
	unsigned short get_source_port(std::vector<char> buffer);
	unsigned short get_destination_port(std::vector<char> buffer);
	
};

