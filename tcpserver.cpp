#include "tcpserver.h"
 
TCPServer::TCPServer(std::string ipaddress, int port):ipaddress_(ipaddress), port_(port)
{

}

void TCPServer::init()
{
	ioService_ = std::make_shared<boost::asio::io_service>();
	work_ = std::make_shared<boost::asio::io_service::work>(*ioService_);
}

void TCPServer::bind()
{
}

bool TCPServer::start()
{
	isRunning_ = true;

	threadGroup_.create_thread(std::bind(&TCPServer::workerThread, this, ioService_));

    boost::asio::ip::tcp::endpoint endpoint = getEndPoint(ipaddress_, std::to_string(port_));

    acceptor_ = std::make_shared<boost::asio::ip::tcp::acceptor>(*ioService_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_));

	std::shared_ptr<boost::asio::ip::tcp::socket> temp = std::make_shared<boost::asio::ip::tcp::socket>(*ioService_);

	boost::system::error_code ec;
	acceptor_->listen(boost::asio::socket_base::max_connections, ec);
	if (ec)
	{
		// An error occurred.
	}

	acceptor_->async_accept(*temp, boost::bind(&TCPServer::handle_accept, this, temp, boost::asio::placeholders::error));
	return true;
}

boost::asio::ip::tcp::endpoint TCPServer::getEndPoint(std::string ipaddress, std::string port_number)
{
    boost::asio::ip::tcp::resolver resolver(*ioService_);
    boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), ipaddress, port_number);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    return endpoint;
}
bool TCPServer::is_running() const
{
	return isRunning_;
}

bool TCPServer::stop()
{
	isRunning_ = false;
	return true;
}




void TCPServer::handle_read(const boost::system::error_code & erro, size_t bytes_transferred)
{

}

void TCPServer::handle_accept(std::shared_ptr<boost::asio::ip::tcp::socket> client, const boost::system::error_code & error)
{
    mtx_accept.lock();
        while (true)
        {
            printf("new client added\n");
            //getting remote ip address
            boost::asio::ip::tcp::endpoint remote_ep = client->remote_endpoint();
            boost::asio::ip::address remote_ad = remote_ep.address();

            std::string client_id = get_new_client_id();
            std::map<std::string, std::shared_ptr<TCPClientEntity>>::iterator it = connection_map_.find(client_id);
            if (it == connection_map_.end())
            {
                printf("remote ipaddress is %s  connected new id is: %s",  remote_ad.to_string().c_str(), client_id.c_str());
                std::shared_ptr<TCPClientEntity> tcpclient = std::make_shared<TCPClientEntity>(client_id, client);
                connectionsmap_.emplace(std::make_pair(client_id, tcpclient));

                newclientconnected_(client_id);

                client->async_receive(boost::asio::buffer(tcpclient->receiveBuffer_.data(), tcpclient->receiveBuffer_.size()),
                    boost::bind(&TCPServer::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, tcpclient, client_id));

                std::shared_ptr<boost::asio::ip::tcp::socket> temp = std::make_shared<boost::asio::ip::tcp::socket>(*ioService_);

                acceptor_->async_accept(*temp, boost::bind(&TCPServer::handle_accept, this, temp, boost::asio::placeholders::error));
                break;
            }
        }
        mtx_accept.unlock();
}

void TCPServer::workerThread(std::shared_ptr<boost::asio::io_service> ioService)
{
	try
	{
		while (isRunning_)
		{
			boost::system::error_code ec;
			ioService->run(ec);
		}
	}
	catch (std::exception& ex)
	{

	}
}
