#include "RawTCPServer.h"

void RawTCPServer::bind_server(int portnumber)
{
	is_running_ = false;
	init_net(portnumber);
}

boost::signals2::connection RawTCPServer::connect_on_data_received(std::function<void(std::string id, char* data, size_t size)> func)
{
	return receive_connection_.connect(func);
}

void RawTCPServer::init_net(int port_number)
{
	WSADATA w;
	SOCKADDR_IN sa;
	DWORD bytes;
	char hostname[HOSTNAME_LEN] = "localhost";
	struct hostent* h;
	unsigned int opt = 1;

	if (WSAStartup(MAKEWORD(2, 2), &w) != 0)
		die("WSAStartup failed\n");

	if ((s0k = socket(AF_INET, SOCK_RAW, IPPROTO_IP)) == INVALID_SOCKET)
		die("unable to open raw socket\n");

	// use default interface
	if ((gethostname(hostname, HOSTNAME_LEN)) == SOCKET_ERROR)
		die("unable to gethostname\n");

	if ((h = gethostbyname(hostname)) == NULL)
		die("unable to gethostbyname\n");

	sa.sin_family = AF_INET;
	sa.sin_port = htons(port_number);
	memcpy(&sa.sin_addr.S_un.S_addr, h->h_addr_list[0], h->h_length);

	if ((bind(s0k, (SOCKADDR*)& sa, sizeof(sa))) == SOCKET_ERROR)
		die("unable to bind() socket\n");
	/* -d on the command line to disable promiscuous mode */
	if ((WSAIoctl(s0k, SIO_RCVALL, &opt, sizeof(opt), NULL, 0, &bytes, NULL, NULL)) == SOCKET_ERROR)
		die("failed to set promiscuous mode\n");
	is_running_ = true;
	threadGroup_.create_thread(std::bind(&RawTCPServer::receive_thread, this));
}

void RawTCPServer::die(const char*)
{
	WSACleanup();
	WriteData("%s");
}

void RawTCPServer::WriteData(const char* buffer)
{
	printf(buffer);
}


void RawTCPServer::receive_thread()
{
	char pak[PAKSIZE];
	DWORD bytes;
	while (1)
	{
		memset(pak, 0, sizeof(pak));
		if ((bytes = recv(s0k, pak, sizeof(pak), 0)) == SOCKET_ERROR)
		{
			die("TCP Server Raw Socket  error on recv\n");
		}
		else {
			receive_connection_("", pak, bytes);
		}
	}
 
}

