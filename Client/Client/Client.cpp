// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#pragma warning( disable : 4996)

#include <cstdio>
#include <iostream>
#include <string>
#include <memory>
#include <vector> 
#include <random>
#include <algorithm>

#include "TCPClient.h"
#include "RawTCPClient.h"

#include "TCPRawPacketEncoder.h"
#include "TCPRawPacketDecoder.h"


std::map<std::string, std::shared_ptr<RawTCPClient>> clients;
std::shared_ptr<TCPClient> server_communication_;


void client_data_received(std::string id, const char* data, size_t size)
{
	printf("Receive %d bytes from target_cleint", size);
	server_communication_->send(data, size);
}
std::string get_hex(unsigned char value)
{
	std::stringstream sstream;
	sstream << std::hex << static_cast<int>(value);
	std::string result = sstream.str();
	for (int i = result.size(); i < 2; i++)
		result = "0" + result;
	result = result.substr(0, 2);
	return result;
}
std::string get_new_id()
{
	char dst[37];
	size_t len = 37;
	int n = snprintf(dst, len, "%08x-%04x-%04x-%04x-%04x%08x",
		rand(),                         // Generates a 32-bit Hex number
		rand() & 0xffff,                // Generates a 16-bit Hex number
		((rand() & 0x0fff) | 0x4000),   // Generates a 16-bit Hex number of the form 4xxx (4 indicates the UUID version)
		(rand() & 0x3fff) + 0x8000,     // Generates a 16-bit Hex number in the range [0x8000, 0xbfff]
		rand() & 0xffff, rand());     // Generates a 48-bit Hex number
	std::string id(dst, 37);
	return id;
}

void print_hex(std::vector<unsigned char> buffer)
{
	std::string value = "";
	for (unsigned char c : buffer)
	{
		value += get_hex(c) + "-";
	}
	printf(value.c_str());
	printf("\n");
}
void print_hex(std::vector<char> buffer)
{
	std::string value = "";
	for (char c : buffer)
	{
		value += get_hex(c) + "-";
	}
	printf(value.c_str());
	printf("\n");
}
std::vector<char> unhex(std::string hex_str)
{
	std::vector<char> temp;
	for (int i = 0; i < hex_str.size(); i += 2)
	{
		std::string hexstr = hex_str.substr(i, 2);

		std::stringstream ss;
		uint8_t x;
		unsigned tmp;

		ss << hexstr;
		ss >> std::hex >> tmp;
		x = tmp;

		temp.push_back(x);
	}
	return temp;
}
int count = 0;
void data_received(std::string id, const char* data, size_t size)
{
	std::shared_ptr<TCPRawPacketEncoder> encoder = std::make_shared<TCPRawPacketEncoder>();
	std::shared_ptr<TCPRawPacketDecoder> decoder = std::make_shared<TCPRawPacketDecoder>(encoder);
	std::vector<char> out_putbuffer;
	std::string raw_str(data, size);
	printf("-----------------------------------------------------------------------------------------------------------------------------------------\n");
	//printf("Raw Packet is \n");


	std::vector<char> input_buffer(size - 4);//skip 4 first bytes
	std::copy(data + 4, data + size, input_buffer.data());//skip 4 first bytes 
	//print_hex(input_buffer);

	//Test Scenario
	//count++;
	//if (count == 1)
	//	input_buffer = unhex("450200342f8b40008006d97fc0a83801c0a83865c73715a850f240b20000000080c2faf013240000020405b40103030801010402");
	//else
	//	return;
	//
	TCPRawPacketEncoder::IPHeader iph = encoder->get_ipheader(input_buffer);
		 
	switch (iph.ip_protocol) //Check the Protocol and do accordingly...
	{
	case 1: //ICMP Protocol 
		printf("ICMP Packet Received\n");
		break;
	case 6: //TCP Protocol
		printf("TCP Packet Received\n");
		break;
	case 17: //UDP Protocol
		printf("UDP Packet Received\n");
		break;
	default: //Some Other Protocol like ARP etc.
		break;
	}

	TCPRawPacketEncoder::TCPHeader tcph = encoder->get_tcp_header(input_buffer);

	//std::string src_ipaddress = encoder->get_source_ipaddress(input_buffer);
	//unsigned short src_port = encoder->get_source_port(input_buffer);
	//std::string dest_ipaddress = encoder->get_destination_ipaddress(input_buffer);
	//unsigned short dest_port = encoder->get_destination_port(input_buffer);
	//std::string str_dest_port = std::to_string(dest_port);

	std::string src_ipaddress ="192.168.56.1";
	unsigned short src_port = 50999;
	std::string dest_ipaddress = "192.168.56.101";
	unsigned short dest_port = 5544;
	std::string str_dest_port = std::to_string(dest_port);
	//TODO:
	if (tcph.syn== 1)//if this is sync packet then change the parameter otherwise skip
	{
		iph.ip_ttl = 128; //Time to Alive
		iph.ip_tos = 2; // Type of Server,  2=ECN-Capable Transport codepoint 0=Not Capable transport codepoint
		iph.ip_id = 0x8520;
		tcph.fin = 0;
		tcph.syn = 0;
		tcph.rst = 0;
		tcph.psh = 0;
		tcph.ack = 0;
		tcph.urg = 0;
		tcph.ecn = 0;
		tcph.cwr = 0;
	
		tcph.syn = 1;
		tcph.ecn = 1;
		tcph.cwr = 1;
		
		tcph.ns =0;
		tcph.reserved_part1 = 0;
		tcph.data_offset =8;
	
		iph.ip_srcaddr = inet_addr("192.168.56.1");
		//iph.ip_id = 0x8b2f;
		iph.ip_checksum = 0x0000;
		iph.ip_total_length = 52; // it should remote other tcp optional data and total_length is (ip header) size +  (tcp header) size

		tcph.source_port = 0xbbd3;
		tcph.dest_port = 0xa815;
		tcph.checksum = 0x0000;
		tcph.sequence = 0xbaf62b25;
		tcph.acknowledge = 0x00;
		std::vector<char> otherinfo = unhex("020405b40103030801010402");
		//tcph.extra_data.insert(std::end(tcph.extra_data), std::begin(otherinfo), std::end(otherinfo));
		std::vector<char> temp = decoder->decode(iph, tcph, otherinfo);
		 
		//tcph.checksum = 0x1324;
	 
		//temp.insert(std::end(temp), std::begin(otherinfo), std::end(otherinfo));
		out_putbuffer = temp;
		printf("New Packet is \n");
		print_hex(out_putbuffer);
	}
	//tcph.
	//


	printf("Destination ip address is : %s\n", dest_ipaddress.c_str());
	printf("Source Port is : %d\n", src_port);
	printf("Destination Port is : %d\n", dest_port);

	//std::vector<char> out_putbuffer(input_buffer.begin(), input_buffer.end());
	//out_putbuffer = decoder->decode(input_buffer, "192.168.56.1", "5544", dest_ipaddress, str_dest_port);
	//printf("Raw Packet After Decoding is : \n");
	//print_hex(out_putbuffer);
	//std::string new_src_ipaddress = encoder->get_source_ipaddress(out_putbuffer);
	//printf("new source ip address is : %s\n", new_src_ipaddress.c_str());
	printf("-----------------------------------------------------------------------------------------------------------------------------------------\n");

	//if (clients.size() <= 0)
	//{
	std::string client_id = get_new_id();
	std::shared_ptr<RawTCPClient> client = std::make_shared<RawTCPClient>(client_id, dest_ipaddress, str_dest_port);
	client->connect_on_data_received(std::bind(client_data_received, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	client->connect_to_server();

	//out_putbuffer.insert(std::begin(out_putbuffer), data, data + 4);
	client->send_buffer(out_putbuffer.data(), out_putbuffer.size());
	//client->send_buffer(data+4, size-4);

	clients.emplace(std::pair<std::string, std::shared_ptr<RawTCPClient>>(client_id, client));
	//}
}

int wmain(int argc, wchar_t* argv[])
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
