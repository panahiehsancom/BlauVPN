#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "TCPRawPacketEncoder.h"

class TCPRawPacketDecoder
{
public:
	TCPRawPacketDecoder(std::shared_ptr<TCPRawPacketEncoder> encoder);
	std::vector<char> decode(std::vector<char> tcpbuffer, std::string sourceip, std::string sourceport, std::string destip, std::string destport);

private:
	std::shared_ptr<TCPRawPacketEncoder> encoder_;
	USHORT CheckSum(USHORT* buffer, int size);
};

