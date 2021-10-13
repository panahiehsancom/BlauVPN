#include "TCPRawPacketDecoder.h"

TCPRawPacketDecoder::TCPRawPacketDecoder(std::shared_ptr<TCPRawPacketEncoder> encoder) :encoder_(encoder)
{
}

std::vector<char> TCPRawPacketDecoder::decode(std::vector<char> tcpbuffer, std::string sourceip, std::string sourceport, std::string destip, std::string destport)
{
	TCPRawPacketEncoder::IPHeader header = encoder_->get_ipheader(tcpbuffer);

	header.ip_srcaddr = inet_addr(sourceip.c_str());
	header.ip_checksum = (unsigned short)CheckSum((USHORT*)&header, sizeof(header));

	std::vector<char> newmemory(sizeof(header));
	memcpy(newmemory.data(), &header, sizeof(header));
	
	for (int i = 0; i < newmemory.size(); i++)
	{
		tcpbuffer[i] = newmemory[i];
	}

	return tcpbuffer;
}

USHORT TCPRawPacketDecoder::CheckSum(USHORT* buffer, int size)
{
	unsigned long cksum = 0;
	while (size > 1)
	{
		cksum += *buffer++;
		size -= sizeof(USHORT);
	}
	if (size)
		cksum += *(UCHAR*)buffer;

	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	return (USHORT)(~cksum);
}
