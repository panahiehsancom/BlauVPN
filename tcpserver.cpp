#include "TCPServer.h"
 
TCPServer::TCPServer(IPeripheralConnection::LanAddress & bindAddress)
	: bindAddress_(bindAddress)
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

	boost::asio::ip::tcp::endpoint endpoint = getEndPoint(bindAddress_);

	acceptor_ = std::make_shared<boost::asio::ip::tcp::acceptor>(*ioService_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), bindAddress_.port_));

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

bool TCPServer::is_running() const
{
	return isRunning_;
}

bool TCPServer::stop()
{
	isRunning_ = false;
	return true;
}

std::shared_ptr<IMessageExtractor> TCPServer::get_extractor(std::string client_id)
{
	std::shared_ptr<PeripheralClient> client = connectionsmap_[client_id];
	return client->get_extractor();
}

boost::signals2::connection TCPServer::connect_on_client_add(std::function<void(std::string client_id)> func)
{
	return  newclientconnected_.connect(func);
}

boost::signals2::connection TCPServer::connect_on_client_removed(std::function<void(std::shared_ptr<PeripheralClient>)> func)
{
	return clientdisconnected_.connect(func);
}

void TCPServer::disconnect_client(std::shared_ptr<PeripheralClient> client)
{

}



void TCPServer::send(char * buffer, size_t size, std::string client_id)
{
	messagePackaged(buffer, size, client_id);
}


void TCPServer::send(std::shared_ptr<ISerializableMessage> msg, std::string client_id)
{
	mtx.lock();

	std::string message_type = convert_message_type(msg->get_type());
	CF::take_log(OBFUSCATED("sending message ") + message_type + OBFUSCATED(" to client:") + client_id);

	std::map<std::string, std::shared_ptr<PeripheralClient>>::iterator it = connectionsmap_.find(client_id);
	if (it != connectionsmap_.end())
	{
		std::shared_ptr<PeripheralClient> client = it->second;
		if (client != nullptr)
		{
			std::shared_ptr<IMessagePackager> packager = client->get_packager();
			packager->package(msg, client_id);
		}
	}
	mtx.unlock();
}

void TCPServer::handle_read(const boost::system::error_code & erro, size_t bytes_transferred, std::shared_ptr<PeripheralClient> client, std::string clientid)
{
	mtx_r.lock();
	if ((boost::asio::error::eof == erro) || (boost::asio::error::connection_reset == erro))
	{
#ifdef DebugEnc
		CF::take_log(OBFUSCATED("error in receiving data so .... deleteing connection from map NetID:") + clientid);
#endif
		std::string id = client->get_client_id();
		clientdisconnected_(client);
		connectionsmap_.erase(id);
	}
	else
	{
		if (bytes_transferred > 0)
		{
			//std::cout << "data recevied" << std::endl;
#ifdef DebugEnc
			CF::take_log(OBFUSCATED("receiving data from NetID:") + clientid);
#endif
			std::map<std::string, std::shared_ptr<PeripheralClient>>::iterator it = connectionsmap_.find(clientid);
			if (it != connectionsmap_.end())
			{
				CF::take_log(OBFUSCATED("Socket Receive :"));
				std::string mystr = "";
				for (int i = 0; i < bytes_transferred; ++i)
				{
					std::stringstream ss;
					ss << std::hex << (int)(unsigned char)client->receiveBuffer_[i];
					std::string tempval(ss.str());
					if (tempval.size() == 1)
						tempval = "0" + tempval;
					mystr += tempval;
				}
				CF::take_log(mystr);

				std::shared_ptr<PeripheralClient> client = it->second;
				std::shared_ptr<IMessageExtractor> extractor = client->get_extractor(); 
				extractor->date_received(client->receiveBuffer_.data(), bytes_transferred, clientid);
				std::shared_ptr<boost::asio::ip::tcp::socket> socket = client->get_socket();
				socket->async_receive(boost::asio::buffer(client->receiveBuffer_.data(), client->receiveBuffer_.size()),
					boost::bind(&TCPServer::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, client, clientid));
		
			}

		}
	}

	mtx_r.unlock();
}

void TCPServer::handle_accept(std::shared_ptr<boost::asio::ip::tcp::socket> client, const boost::system::error_code & error)
{
	mtx_accept.lock();
	while (true)
	{
		std::cout << OBFUSCATED("new client added") << std::endl;
		//getting remote ip address
		boost::asio::ip::tcp::endpoint remote_ep = client->remote_endpoint();
		boost::asio::ip::address remote_ad = remote_ep.address();

		std::string client_id = get_new_client_id();
		std::map<std::string, std::shared_ptr<PeripheralClient>>::iterator it = connectionsmap_.find(client_id);
		if (it == connectionsmap_.end())
		{
			CF::take_log(OBFUSCATED("remote ipaddress is ") + remote_ad.to_string() + OBFUSCATED(" connected new id is:") + client_id);
			std::shared_ptr<PeripheralClient> tcpclient = std::make_shared<PeripheralClient>(client_id, client);
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

void TCPServer::messagePackaged(const char * buffer, size_t size, std::string client_id)
{
	std::map<std::string, std::shared_ptr<PeripheralClient>>::iterator it = connectionsmap_.find(client_id);
	if (it != connectionsmap_.end())
	{
		std::shared_ptr<PeripheralClient> client = it->second;
		std::shared_ptr<boost::asio::ip::tcp::socket> client_socket = client->get_socket();
		client_socket->send(boost::asio::buffer(buffer, size));
	}
}

boost::asio::ip::tcp::endpoint TCPServer::getEndPoint(IPeripheralConnection::LanAddress address)
{
	boost::asio::ip::tcp::resolver resolver(*ioService_);
	boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), address.ipaddress_, std::to_string(address.port_));
	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
	return endpoint;
}

std::string TCPServer::get_new_client_id()
{
	mtx_id.lock();
	GUID guid;
	CoCreateGuid(&guid);

	std::ostringstream os;
	os << std::hex << std::setw(8) << std::setfill('0') << guid.Data1;
	os << '-';
	os << std::hex << std::setw(4) << std::setfill('0') << guid.Data2;
	os << '-';
	os << std::hex << std::setw(4) << std::setfill('0') << guid.Data3;
	os << '-';
	os << std::hex << std::setw(2) << std::setfill('0') << static_cast<short>(guid.Data4[0]);
	os << std::hex << std::setw(2) << std::setfill('0') << static_cast<short>(guid.Data4[1]);
	os << '-';
	os << std::hex << std::setw(2) << std::setfill('0') << static_cast<short>(guid.Data4[2]);
	os << std::hex << std::setw(2) << std::setfill('0') << static_cast<short>(guid.Data4[3]);
	os << std::hex << std::setw(2) << std::setfill('0') << static_cast<short>(guid.Data4[4]);
	os << std::hex << std::setw(2) << std::setfill('0') << static_cast<short>(guid.Data4[5]);
	os << std::hex << std::setw(2) << std::setfill('0') << static_cast<short>(guid.Data4[6]);
	os << std::hex << std::setw(2) << std::setfill('0') << static_cast<short>(guid.Data4[7]);

	std::string s(os.str());
	mtx_id.unlock();
	return s;
}

std::string TCPServer::convert_message_type(IMessage::MessageType type)
{
	switch (type)
	{
	case IMessage::PublicKey:
		return OBFUSCATED("PublicKey");
	case IMessage::Identification:
		return OBFUSCATED("Identification");
	case IMessage::Config:
		return OBFUSCATED("Config");
 	case IMessage::SessionKey:
		return OBFUSCATED("SessionKey");
	case IMessage::JobFinished:
		return OBFUSCATED("JobFinished");
	case IMessage::Abort:
		return OBFUSCATED("Abort");
	case IMessage::GetClientList:
		return OBFUSCATED("GetClientList");
	case IMessage::ClientList:
		return OBFUSCATED("ClientList");
	case IMessage::GetClientInformation:
		return OBFUSCATED("GetClientInformation");
	case IMessage::ClientInformation:
		return OBFUSCATED("ClientInformation");
	case IMessage::Acknowledge:
		return OBFUSCATED("Acknowledge");
	case IMessage::GetIdentification:
		return OBFUSCATED("GetIdentification");
	case IMessage::None:
		return OBFUSCATED("None");
	default:
		return OBFUSCATED("None");
	}
}
