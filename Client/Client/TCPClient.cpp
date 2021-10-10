#include "TCPClient.h"

TCPClient::TCPClient(std::string id) :isRunning_(false), id_(id)
{

	//connection_packager_ = packager_->connectOnMessagePackaged(std::bind(&TC::send, this, std::placeholders::_1, std::placeholders::_2));

	ioService_ = std::make_shared<boost::asio::io_service>();

	work_ = std::make_shared<boost::asio::io_service::work>(*ioService_);
}

void  TCPClient::init()
{

}

bool TCPClient::connect(std::string ipaddress, std::string port)
{
	try
	{
		is_connected_ = false;
		destAddr_.ipaddress_ = ipaddress;
		destAddr_.port_ = std::atoi(port.c_str());
		serverEndPoint_ = getEndPoint(destAddr_);

		socket_ = std::make_shared<boost::asio::ip::tcp::socket>(*ioService_); 

		boost::asio::ip::tcp::resolver resolver(*ioService_);

		boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), destAddr_.ipaddress_, std::to_string(destAddr_.port_));

		boost::asio::ip::tcp::resolver::iterator EndPointIterator = resolver.resolve(query);

		socket_->async_connect(serverEndPoint_,
			boost::bind(&TCPClient::handle_connect, shared_from_this(),
				_1, EndPointIterator));

		threadGroup_.create_thread(std::bind(&TCPClient::workerThread, this, ioService_));
		isRunning_ = true;
		
		return is_connected_;
	}
	catch (boost::exception& ex)
	{
#ifdef SDBG
		CommonFunction::take_log( "exception in connecting" );
		std::string error = boost::diagnostic_information(ex);
		CommonFunction::take_log(error);
#endif //  debug
		return false;
	}
	catch (...)
	{
#ifdef SDBG
		CommonFunction::take_log("exception in connecting");
		
#endif //  debug
		return false;
	}
}

bool TCPClient::connect(LanAddress end_point)
{
	return connect(end_point.ipaddress_, std::to_string(end_point.port_));
}

void TCPClient::connect_again()
{
	boost::asio::ip::tcp::resolver resolver(*ioService_);

	boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), destAddr_.ipaddress_, std::to_string(destAddr_.port_));
	boost::asio::ip::tcp::resolver::iterator EndPointIterator = resolver.resolve(query);
	socket_->async_connect(serverEndPoint_,
		boost::bind(&TCPClient::handle_connect, shared_from_this(),
			_1, ++EndPointIterator));
}

TCPClient::LanAddress TCPClient::get_end_point() const
{
	return LanAddress();
}
 
void TCPClient::disconnect()
{
	isRunning_ = false;
	is_connected_ = false;
	connection_packager_.disconnect();
	boost::system::error_code ec;
	socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
	socket_->close();
	ioService_->stop();
	threadGroup_.join_all();
}

void TCPClient::send(const char* buffer, size_t size)
{
	send_mtx.lock();
	if (isRunning_ && socket_->is_open())
	{
		try
		{
			socket_->send(boost::asio::buffer(buffer, size));
		}
		catch (std::exception const& e)
		{
			isRunning_ = false;
			is_connected_ = false;
		}
		catch (...)
		{
			isRunning_ = false;
			is_connected_ = false;
		}
	}
	send_mtx.unlock();
} 
bool TCPClient::is_connected() const
{
	return is_connected_;
}

std::string TCPClient::get_id() const
{
	return id_;
}

boost::signals2::connection TCPClient::connect_on_data_received(std::function<void(std::string id, const char * data, size_t size)> func)
{
	return on_data_received_signal_.connect(func);
}

void TCPClient::handle_connect(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{

	if (error.value() == 0)
	{
		//CommonFunction::take_log("Connect Successfully to TCP Server");
		is_connected_ = true;
	
		socket_->async_receive(boost::asio::buffer(receiveBuffer_.data(), receiveBuffer_.size()),
			boost::bind(&TCPClient::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
	else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
	{
		std::string message = error.message();
		//CommonFunction::take_log(message);
		socket_->close();
		boost::asio::ip::tcp::endpoint EndPoint = *endpoint_iterator;

		socket_->async_connect(serverEndPoint_,
			boost::bind(&TCPClient::handle_connect, this,
				boost::asio::placeholders::error, ++endpoint_iterator));
	}
}

void TCPClient::handle_read(const boost::system::error_code& erro, size_t bytes_transferred)
{
	try
	{
		if (!erro)
		{ 
			//extractor_->date_received(receiveBuffer_.data(), bytes_transferred);
			on_data_received_signal_(id_, receiveBuffer_.data(), bytes_transferred);
			socket_->async_receive(boost::asio::buffer(receiveBuffer_.data(), receiveBuffer_.size()),
				boost::bind(&TCPClient::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));


		}
		else
		{
			//is_connected_ = false;
			//socket_->close();
		}
	}
	catch (std::exception& ex)
	{
#ifdef SDBG 
		CommonFunction::take_log(ex.what());
#endif  //debug
		//std::cout << ex.what() << std::endl;
		//is_connected_ = false;
		//disconnect();
	}
	catch (...)
	{

	}
}

boost::asio::ip::tcp::endpoint TCPClient::getEndPoint(LanAddress address)
{
	boost::asio::ip::tcp::resolver resolver(*ioService_);
	boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), address.ipaddress_, std::to_string(address.port_));
	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
	return endpoint;
}

void TCPClient::workerThread(std::shared_ptr<boost::asio::io_service> ioService)
{
	try
	{
		while (isRunning_)
		{
			//qDebug() << "IO Service Start";
			boost::system::error_code ec;
			ioService->run(ec);
		}

	}
	catch (std::exception& ex)
	{
		//CommonFunction::take_log(ex.what());
		is_connected_ = false;
	}
}