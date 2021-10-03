#pragma once

#include <functional>
#include <iostream>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/thread.hpp>  
#include <boost/signals2.hpp>

class TCPClient :  public std::enable_shared_from_this<TCPClient>
{
public:

	struct LanAddress {

	public:
		LanAddress() {}
		LanAddress(const std::string ipaddress, const short port) : ipaddress_(ipaddress), port_(port)
		{

		}
		bool operator ==(const LanAddress& lanAddress)
		{
			if (lanAddress.ipaddress_ == ipaddress_ && port_ == lanAddress.port_)
				return true;
			return false;
		}

		std::string ipaddress_;
		short port_;
	};

	TCPClient(std::string id);

	// Inherited via IPeripheralConnection
	void init() ;
	bool connect(std::string ipaddress, std::string port) ;
	bool connect(LanAddress end_point) ;
	void connect_again();
	LanAddress get_end_point() const ;
	void disconnect() ;
	void send(const char* buffer, size_t size) ;
	bool is_connected() const ;
	std::string get_id() const ;
	boost::signals2::connection connect_on_data_received(std::function<void(std::string id, const char* data, size_t size)> func);

private:
	void handle_connect(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
	void handle_read(const boost::system::error_code& erro, size_t bytes_transferred);
	boost::asio::ip::tcp::endpoint getEndPoint(LanAddress address);
	void workerThread(std::shared_ptr<boost::asio::io_service> ioService);
	std::shared_ptr<boost::asio::io_service> ioService_;
	std::shared_ptr<boost::asio::io_service::work> work_;
	std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
	boost::asio::ip::tcp::endpoint serverEndPoint_;
	boost::thread_group threadGroup_;
	boost::asio::streambuf response_;
	boost::signals2::signal<void(size_t size)> receiveDataSignal_;
	boost::signals2::signal<void(std::string id, const char* data, size_t size)> on_data_received_signal_;
	std::mutex mtx_;
	boost::signals2::connection connection_packager_;
	bool isRunning_;
	bool is_connected_;
	std::array<char, 8192> receiveBuffer_;
	std::mutex send_mtx;
	LanAddress destAddr_;
	std::string id_;
};
