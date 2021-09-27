#pragma once

#include <functional>
#include <iostream>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/generic/detail/endpoint.hpp>
#include <boost/signals2.hpp>
#include <boost/thread.hpp>

#include <Windows.h>
#include <ObjBase.h>

#include "IPeripheralListener.h"  
#include "IMessageExtractor.h"
#include "IMessagePackager.h"
#include "MessageExtractor.h"
//#include "peripheralclientfactory.h"
//#include "tcpclient.h"

typedef std::pair<std::shared_ptr<boost::asio::ip::tcp::socket>, std::string> connectionpair;
 
		class TCPServer : public IPeripheralListener
		{
		public:
			TCPServer(IPeripheralConnection::LanAddress& bindAddress);

			// Inherited via IPeripheralListener
			void init() override;
			void bind() override;
			bool start() override;
			bool is_running() const override;
			bool stop() override; 
			std::shared_ptr<IMessageExtractor> get_extractor(std::string client_id);
			boost::signals2::connection connect_on_client_add(std::function<void(std::string client_id)> func) override;
			boost::signals2::connection connect_on_client_removed(std::function<void(std::shared_ptr<PeripheralClient>)> func) override;
			void  disconnect_client(std::shared_ptr<PeripheralClient> client) override;  
			void send(char * buffer, size_t size, std::string client_id) override; 
			void send(std::shared_ptr<ISerializableMessage> msg, std::string client_id) override; 

		private:
			void handle_read(const boost::system::error_code &erro, size_t bytes_transferred, std::shared_ptr<PeripheralClient> client, std::string clientid);
			void handle_accept(std::shared_ptr<boost::asio::ip::tcp::socket> client, const boost::system::error_code& error);
			void workerThread(std::shared_ptr<boost::asio::io_service> ioService);
			void messagePackaged(const char * buffer, size_t size, std::string client_id);
			std::string get_new_client_id();
			std::string convert_message_type(IMessage::MessageType type);
			boost::asio::ip::tcp::endpoint getEndPoint(IPeripheralConnection::LanAddress address); 

			std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
			std::shared_ptr<boost::asio::ip::tcp::socket> client_;
			std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
			std::shared_ptr<boost::asio::io_service::work> work_;
			std::shared_ptr<boost::asio::io_service> ioService_;
			 
			IPeripheralConnection::LanAddress bindAddress_; 
			boost::signals2::signal<void(std::string)> newclientconnected_;
			boost::signals2::signal<void(std::shared_ptr<PeripheralClient> client)> clientdisconnected_; 
			boost::asio::ip::tcp::endpoint serverEndPoint_;
			boost::thread_group threadGroup_; 
			bool isRunning_;  
			std::map<std::string, std::shared_ptr<PeripheralClient>> connectionsmap_;
			std::array<char, 22808> receiveBuffer_; 
			std::mutex mtx;
			std::mutex mtx_r;
			std::mutex mtx_accept;
			std::mutex mtx_id;
		};
 