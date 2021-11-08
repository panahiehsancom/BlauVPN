#include "TCPRawPacketDecoder.h"

TCPRawPacketDecoder::TCPRawPacketDecoder(std::shared_ptr<TCPRawPacketEncoder> encoder) :encoder_(encoder)
{

}

std::vector<char> TCPRawPacketDecoder::decode(std::vector<char> tcpbuffer, std::string sourceip, std::string sourceport, std::string destip, std::string destport)
{
	TCPRawPacketEncoder::IPHeader header = encoder_->get_ipheader(tcpbuffer);

	header.ip_srcaddr = inet_addr(sourceip.c_str());
	header.ip_checksum = (unsigned short)CheckSum((USHORT*)& header, sizeof(header));

	std::vector<char> newmemory(sizeof(header));
	memcpy(newmemory.data(), &header, sizeof(header));

	for (int i = 0; i < newmemory.size(); i++)
	{
		tcpbuffer[i] = newmemory[i];
	}

	return tcpbuffer;
}

std::vector<char> TCPRawPacketDecoder::decode(TCPRawPacketEncoder::IPHeader header, TCPRawPacketEncoder::TCPHeader tcpheader, std::vector<char> extra_tcp)
{
	iphdr s;

	header.ip_checksum = (unsigned short)CheckSum((USHORT*)& header, sizeof(header));

	unsigned short total_len = ntohs(header.ip_total_length);

	int tcpopt_len = tcpheader.data_offset * 4 - 20;
	int tcpdatalen = total_len - (tcpheader.data_offset * 4) - (header.ip_header_len * 4);

	std::vector<unsigned  char> temp;
	char Reserved = 0x00;
	char Protocol = 0x06;
	unsigned short tcp_size = htons(sizeof(tcpheader) + tcpopt_len + tcpdatalen);

	TCPRawPacketEncoder::pseudo_tcp_header psudo_header;
	psudo_header.dest = *(struct in_addr*) & header.ip_destaddr;
	psudo_header.source = *(struct in_addr*) & header.ip_srcaddr;
	psudo_header.protocol = 6;
	psudo_header.reserved = 0; 
	psudo_header.tcp_size = tcp_size;
	int totaltcp_len = sizeof(psudo_header) + sizeof(tcpheader) + tcpopt_len + tcpdatalen;
	unsigned short* tcp = new unsigned short[totaltcp_len];


	memcpy((unsigned char*)tcp, &psudo_header, sizeof(psudo_header));
	memcpy((unsigned char*)tcp + sizeof(psudo_header), (unsigned char*)&tcpheader, sizeof(tcpheader));
	memcpy((unsigned char*)tcp + sizeof(psudo_header) + sizeof(tcpheader), (unsigned char*)&header + (header.ip_header_len * 4) + (sizeof(tcpheader)), tcpopt_len);
	memcpy((unsigned char*)tcp + sizeof(psudo_header) + sizeof(tcpheader) + tcpopt_len, (unsigned char*)&tcpheader + (tcpheader.data_offset * 4), tcpdatalen);

	//temp.insert(std::end(temp), (char*)& header  , (char*)& header + sizeof(header));
	temp.insert(std::end(temp), (unsigned  char*)& psudo_header, (unsigned  char*)& psudo_header + sizeof(psudo_header));
	temp.insert(std::end(temp), (unsigned  char*)& tcpheader, (unsigned  char*)& tcpheader + sizeof(tcpheader));
	temp.insert(std::end(temp), (unsigned  char*)& tcpheader, (unsigned  char*)& tcpheader + sizeof(tcpheader));
	temp.insert(std::end(temp), std::begin(extra_tcp), std::end(extra_tcp));
	tcpheader.checksum = CheckSum(tcp, totaltcp_len);

	//tcpheader.checksum =  tcp_checksum((void*)&temp, temp.size() / 2, (void*)& header.ip_srcaddr, (void*)& header.ip_destaddr);
	tcpheader.checksum = CheckSum((USHORT*)& temp, temp.size() /2);
	//tcpheader.checksum = compute_tcp_checksum(header, tcpheader, extra_tcp);

	std::vector<char> ip_header_buffer(sizeof(header));
	memcpy(ip_header_buffer.data(), &header, sizeof(header));

	std::vector<char> tcp_header_buffer(sizeof(tcpheader));
	memcpy(tcp_header_buffer.data(), &tcpheader, sizeof(tcpheader));

	std::vector<char> final_buffer;

	final_buffer.insert(std::end(final_buffer), std::begin(ip_header_buffer), std::end(ip_header_buffer));
	final_buffer.insert(std::end(final_buffer), std::begin(tcp_header_buffer), std::end(tcp_header_buffer));
	final_buffer.insert(std::end(final_buffer), std::begin(extra_tcp), std::end(extra_tcp));

	return final_buffer;
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

unsigned short TCPRawPacketDecoder::compute_tcp_checksum(TCPRawPacketEncoder::IPHeader ipheader, TCPRawPacketEncoder::TCPHeader tcpheader, std::vector<char> extra_tcp)
{
	unsigned long sum = 0;
	unsigned short tcplen = ntohs(ipheader.ip_total_length) - (ipheader.ip_header_len << 2);
	std::vector<unsigned short> temp;
	//temp.insert(std::end(temp), (char*)& header, (char*)& header + sizeof(header));
	temp.insert(std::end(temp), (unsigned short*)& tcpheader, (unsigned short*)& tcpheader + sizeof(tcpheader));
	const short* data = reinterpret_cast<const short*> (extra_tcp.data());
	std::vector<unsigned short> s(data, data + extra_tcp.size() / 2);
	temp.insert(std::end(temp), std::begin(s), std::end(s));

	//add the pseudo header 
	//the source ip
	sum += (ipheader.ip_srcaddr >> 16) & 0xFFFF;
	sum += (ipheader.ip_srcaddr) & 0xFFFF;
	//the dest ip
	sum += (ipheader.ip_destaddr >> 16) & 0xFFFF;
	sum += (ipheader.ip_destaddr) & 0xFFFF;

	//protocol and reserved: 6
	sum += htons(IPPROTO_TCP);
	//the length
	sum += htons(tcplen);

	//add the IP payload
	//initialize checksum to 0
	tcpheader.checksum = 0;
	int index = 0;
	while (tcplen > 1) {
		sum += *temp.data() + index;
		index++;
		tcplen -= 2;
	}
	sum = ~sum;
	return sum;
}

uint16_t TCPRawPacketDecoder::tcp_checksum(const void* buff, size_t len, void* src_addr, void* dest_addr)
{
	//unsigned long cksum = 0;
	//
	//const uint16_t* buf = (uint16_t*)buff;
	//while (len > 1)
	//{
	//	cksum += *buf++;
	//	len -= sizeof(USHORT);
	//}
	//if (len)
	//	cksum += *(UCHAR*)buf;
	//
	//cksum = (cksum >> 16) + (cksum & 0xffff);
	//cksum += (cksum >> 16);
	//return (USHORT)(~cksum);



	///
	 const uint16_t* buf = (uint16_t* )buff;
	 uint32_t* ip_src  = (uint32_t*)&src_addr, * ip_dst = (uint32_t*)&dest_addr;
	 uint16_t sum;
	size_t length = len;

	// Calculate the sum                                            //
	sum = 0;
	while (len > 1)
	{
		sum += htons(*buf++);
		if (sum & 0x80000000)
			sum = (sum & 0xFFFF) + (sum >> 16);
		len -= 2;
	}

	if (len & 1)
		// Add the padding if the packet lenght is odd          //
		sum += *((uint8_t*)buf);

	// Add the pseudo-header                                        //
	sum += *(ip_src++);
	sum += *ip_src;
	sum += *(ip_dst++);
	sum += *ip_dst;
	sum += htons(IPPROTO_TCP);
	sum += htons(length);

	// Add the carries                                              //
	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	// Return the one's complement of sum                           //
	return ((uint16_t)(~sum));
}
