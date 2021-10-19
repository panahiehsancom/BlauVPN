#pragma once

#pragma warning( disable : 4996)

#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <time.h>

#include <boost/signals2.hpp>
#include <boost/thread.hpp>

#pragma pack(pop)

#define HOSTNAME_LEN 1024
#define SIO_RCVALL _WSAIOW(IOC_VENDOR, 1)
#define PAKSIZE 65536

/* G L O B A L S */
SOCKET s0k;
short promiscuous = 1;
int minSize = 30;

class RawTCPServer
{
public:
	void bind_server(int portnumber);
	boost::signals2::connection connect_on_data_received(std::function<void(std::string id, char* data, size_t size)> func);
	void receive_thread();
private:
	void init_net(int portnumber);
	void die(const char*);
	void WriteData(const char* buffer);
	boost::signals2::signal<void(std::string id, char* data, size_t size)> receive_connection_;

	boost::thread_group threadGroup_;
	bool is_running_;
};

