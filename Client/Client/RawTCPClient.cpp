#include "RawTCPClient.h"

RawTCPClient::RawTCPClient(std::string id, std::string ipaddress, std::string port_number) : id_(id), ipaddress_(ipaddress), port_(port_number)
{
	is_running_ = false;
}

int RawTCPClient::connect_to_server()
{
	int optval;

	WSADATA wsaData;
	ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	const char* sendbuf = "this is a test";
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Validate the parameters
	//if (argc != 2) {
	//	printf("usage: %s server-name\n", argv[0]);
	//	return 1;
	//}

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_RAW;

	// Resolve the server address and port
	iResult = getaddrinfo(ipaddress_.c_str(), port_.c_str(), &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);

		if (ConnectSocket == SOCKET_ERROR)
		{
			printf("Creation of raw socket failed.");
			return 0;
		}
		//Put Socket in RAW Mode.
		printf("\nSetting the socket in RAW mode...");
		if (setsockopt(ConnectSocket, IPPROTO_IP, IP_HDRINCL, (char*)& optval, sizeof(optval)) == SOCKET_ERROR)
		{
			printf("failed to set socket in raw mode.");
			return 0;
		}
		printf("Successful.");

		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	is_running_ = true;
}

int RawTCPClient::send_buffer(const char* data, size_t size)
{ 
	// Send an initial buffer
	int iResult = send(ConnectSocket, data, size, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

}

boost::signals2::connection RawTCPClient::connect_on_data_received(std::function<void(std::string id, char* data, size_t size)> func)
{
	return receive_connection_.connect(func);
}

void RawTCPClient::receive_thread()
{
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	int iResult;
	// Receive until the peer closes the connection
	while(is_running_)
	{
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			printf("Bytes received: %d\n", iResult);
			receive_connection_(id_, recvbuf, recvbuflen);
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());

	}  
}
