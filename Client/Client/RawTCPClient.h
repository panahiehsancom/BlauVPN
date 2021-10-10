#pragma once

#include <functional>
#include <string>

#include <boost/signals2.hpp>


#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512 


class RawTCPClient
{

public:
	RawTCPClient(std::string id, std::string ipaddress, std::string port_number);
	int connect_to_server();
	int send_buffer(const char* data, size_t size);
	boost::signals2::connection connect_on_data_received(std::function<void(std::string id, char* data, size_t size)> func);

private:
	boost::signals2::signal<void(std::string id, char* data, size_t size)> receive_connection_;
	std::string id_;
	std::string ipaddress_;
	std::string port_;

	void receive_thread();
	SOCKET ConnectSocket;
};

