#include "TCPRawPacketDecoder.h"

TCPRawPacketDecoder::TCPRawPacketDecoder(std::shared_ptr<TCPRawPacketEncoder> encoder) :encoder_(encoder)
{

}

std::vector<char> TCPRawPacketDecoder::decode(std::vector<char> tcpbuffer, std::string sourceip, std::string sourceport, std::string destip, std::string destport)
{
	TCPRawPacketEncoder::iphdr header = encoder_->get_ipheader(tcpbuffer);

	header.saddr = inet_addr(sourceip.c_str());
	header.check = (unsigned short)CheckSum((USHORT*)& header, sizeof(header));

	std::vector<char> newmemory(sizeof(header));
	memcpy(newmemory.data(), &header, sizeof(header));

	for (int i = 0; i < newmemory.size(); i++)
	{
		tcpbuffer[i] = newmemory[i];
	}

	return tcpbuffer;
}

std::vector<char> TCPRawPacketDecoder::decode(std::shared_ptr<TCPRawPacketEncoder::iphdr> header, std::shared_ptr<TCPRawPacketEncoder::tcphdr> tcpheader, std::vector<char> extra_tcp)
{

	header->check = (unsigned short)CheckSum((USHORT*)header.get(), sizeof(header));

	tcpheader->check = 0;
	tcpheader->check = compute_tcp_checksum(header, tcpheader);



	//std::vector<char> ip_header_buffer(sizeof(header));
	//memcpy(ip_header_buffer.data(), (char *)&header, sizeof(TCPRawPacketEncoder::tcphdr));
	//
	//std::vector<char> tcp_header_buffer(sizeof(tcpheader));
	//memcpy(tcp_header_buffer.data(), (char*)&tcpheader, sizeof(TCPRawPacketEncoder::tcphdr));

	std::vector<char> final_buffer;

	final_buffer.insert(std::end(final_buffer), (unsigned char*)&header, (unsigned char*)&header + sizeof(struct TCPRawPacketEncoder::iphdr));
	final_buffer.insert(std::end(final_buffer), (unsigned char*)&tcpheader, (unsigned char*)&tcpheader + sizeof(struct TCPRawPacketEncoder::tcphdr));
	final_buffer.insert(std::end(final_buffer), std::begin(extra_tcp), std::end(extra_tcp));

	return final_buffer;
}

unsigned short TCPRawPacketDecoder::CheckSum(USHORT* buffer, int size)
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

unsigned short TCPRawPacketDecoder::compute_tcp_checksum(std::shared_ptr<TCPRawPacketEncoder::iphdr> myip, std::shared_ptr <TCPRawPacketEncoder::tcphdr> mytcp)
{
	unsigned short  total_len = ntohs(myip->tot_len);

	int tcpopt_len = mytcp->doff * 4 - 20;
	int tcpdatalen = total_len - (mytcp->doff * 4) - (myip->ihl * 4);
	TCPRawPacketEncoder::tcp_pseudo pseudohead;
	mytcp->check = 0;
	pseudohead.src_addr = myip->saddr;
	pseudohead.dst_addr = myip->daddr;
	pseudohead.zero = 0;
	pseudohead.proto = IPPROTO_TCP;
	pseudohead.length = htons(sizeof(struct TCPRawPacketEncoder::tcphdr) + tcpopt_len + tcpdatalen);

	int totaltcp_len = sizeof(struct TCPRawPacketEncoder::tcp_pseudo) + sizeof(struct TCPRawPacketEncoder::tcphdr) + tcpopt_len + tcpdatalen;
	unsigned short* tcp = new unsigned short[totaltcp_len];
	std::vector<unsigned char > test;
	size_t size_tcp_header_struct = sizeof(struct TCPRawPacketEncoder::tcphdr);
	size_t  size_tcp_pseudo = sizeof(struct TCPRawPacketEncoder::tcp_pseudo);
	size_t temp_size = (myip->ihl * 4);
	size_t temp2_size = (mytcp->doff * 4);
	test.insert(std::end(test), (unsigned char*)&pseudohead, (unsigned char*)& pseudohead + size_tcp_pseudo);
	test.insert(std::end(test), (unsigned char*)&mytcp, (unsigned char*)&mytcp + size_tcp_header_struct);
	test.insert(std::end(test), (unsigned char*)&myip + (temp_size + size_tcp_header_struct), (unsigned char*)&myip + (temp_size + size_tcp_header_struct + tcpdatalen));
	if(tcpdatalen > temp2_size)//most of time tcp data len is zero
		test.insert(std::end(test), (unsigned char*)&mytcp + temp2_size, (unsigned char*)&mytcp + tcpdatalen);
	memset(tcp, totaltcp_len, 0);
	memcpy((unsigned char*)tcp, &pseudohead, sizeof(struct TCPRawPacketEncoder::tcp_pseudo));
	memcpy((unsigned char*)tcp + sizeof(struct TCPRawPacketEncoder::tcp_pseudo), (unsigned char*)& mytcp, sizeof(struct TCPRawPacketEncoder::tcphdr));
	memcpy((unsigned char*)tcp + sizeof(struct TCPRawPacketEncoder::tcp_pseudo) + sizeof(struct TCPRawPacketEncoder::tcphdr), (unsigned char*)& myip + (myip->ihl * 4) + (sizeof(struct TCPRawPacketEncoder::tcphdr)), tcpopt_len);
	memcpy((unsigned char*)tcp + sizeof(struct TCPRawPacketEncoder::tcp_pseudo) + sizeof(struct TCPRawPacketEncoder::tcphdr) + tcpopt_len, (unsigned char*)& mytcp + (mytcp->doff * 4), tcpdatalen);


	//std::vector<unsigned char > test(totaltcp_len*2);
	//std::copy(tcp, tcp + totaltcp_len*2, test.data());
	return CheckSum(tcp, totaltcp_len);
	//return CheckSum((unsigned short* )test.data(), test.size());
}
