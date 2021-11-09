#include "TCPRawPacketEncoder.h"

TCPRawPacketEncoder::iphdr TCPRawPacketEncoder::get_ipheader(std::vector<char> buffer)
{
	iphdr iph = *((iphdr*)buffer.data()); 
	return iph;
}

 TCPRawPacketEncoder::tcphdr TCPRawPacketEncoder::get_tcp_header(std::vector<char> buffer)
{
	std::vector<unsigned char> tcpheader_buffer(buffer.data() + 20, buffer.data() + 40);
	tcphdr  tcph = *((tcphdr*)(tcpheader_buffer.data())); 
	return tcph;
}

std::string TCPRawPacketEncoder::get_source_ipaddress(std::vector<char> buffer)
{
	TCPRawPacketEncoder::iphdr iph = get_ipheader(buffer);
	struct in_addr src_ip_addr;
	src_ip_addr.s_addr = iph.saddr;
	const char * sourceip = inet_ntoa(src_ip_addr);
	std::string str_ip = sourceip;
	return str_ip;
}

std::string TCPRawPacketEncoder::get_destination_ipaddress(std::vector<char> buffer)
{
	TCPRawPacketEncoder::iphdr iph = get_ipheader(buffer);
	struct in_addr dest_ip_addr;
	dest_ip_addr.s_addr = iph.daddr;
	const char* dest_ip = inet_ntoa(dest_ip_addr);
	std::string str_ip = dest_ip;
	return str_ip;
}

unsigned short TCPRawPacketEncoder::get_source_port(std::vector<char> buffer)
{
	std::vector<unsigned char> tcpheader_buffer(buffer.data() + 20, buffer.data() + 40);
	std::vector<char> source_port_buffer(tcpheader_buffer.data(), tcpheader_buffer.data() + 2); 
	std::reverse(source_port_buffer.begin(), source_port_buffer.end());
	unsigned short src_port = *((unsigned short*)source_port_buffer.data());
	return src_port;
}

unsigned short TCPRawPacketEncoder::get_destination_port(std::vector<char> buffer)
{
	std::vector<unsigned char> tcpheader_buffer(buffer.data() + 20, buffer.data() + 40); 
	std::vector<char> dest_port_buffer(tcpheader_buffer.data() + 2, tcpheader_buffer.data() + 4);
	std::reverse(dest_port_buffer.begin(), dest_port_buffer.end());
	unsigned short dest_port = *((unsigned short*)dest_port_buffer.data());
	return dest_port;
}
