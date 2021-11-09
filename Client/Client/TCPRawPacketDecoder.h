#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <cstring>
#include "TCPRawPacketEncoder.h"

class TCPRawPacketDecoder
{
public:
	TCPRawPacketDecoder(std::shared_ptr<TCPRawPacketEncoder> encoder);
	std::vector<char> decode(std::vector<char> tcpbuffer, std::string sourceip, std::string sourceport, std::string destip, std::string destport);
	std::vector<char> decode(std::shared_ptr<TCPRawPacketEncoder::iphdr> header, std::shared_ptr<TCPRawPacketEncoder::tcphdr> tcpheader, std::vector<char> extra_tcp);


private:
	std::shared_ptr<TCPRawPacketEncoder> encoder_;
	USHORT CheckSum(USHORT* buffer, int size);
	unsigned short compute_tcp_checksum(std::shared_ptr<TCPRawPacketEncoder::iphdr> ipheader, std::shared_ptr<TCPRawPacketEncoder::tcphdr> tcpheader);
	uint16_t tcp_checksum(const void* buff, size_t len, void * src_addr, void * dest_addr);
};

