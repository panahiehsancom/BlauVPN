// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <cstdio>
#include <iostream>
#include <string>
#include <memory>

#include "TCPClient.h"

void data_received(std::string id, const char* data, size_t size)
{

}
int wmain(int argc, wchar_t * argv[])
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
		std::shared_ptr<TCPClient> client = std::make_shared<TCPClient>("unknown_id");
		client->connect_on_data_received(std::bind(data_received, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		client->connect(str_ip, str_port);
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
