#pragma once

#include <functional>
#include <iostream>
#include <memory>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/generic/detail/endpoint.hpp>
#include <boost/signals2.hpp>
#include <boost/thread.hpp>

typedef std::pair<std::shared_ptr<boost::asio::ip::tcp::socket>, std::string> connectionpair;
 
        class TCPServer
		{
		public:
            struct TCPClientEntity
            {
            public:
                TCPClientEntity(std::string id, std::shared_ptr<boost::asio::ip::tcp::socket> client, std::array<char, 22808> array)
                {
                    id_ = id;
                    client_ = client;
                    receiveBuffer_ =array;
                }
                std::string id_;
                std::array<char, 22808> receiveBuffer_;
                std::shared_ptr<boost::asio::ip::tcp::socket> get_socket()
                {
                    return client_;
                }
            private:
                std::shared_ptr<boost::asio::ip::tcp::socket> client_;
            };
            TCPServer(std::string ipaddress, int port);
			// Inherited via IPeripheralListener
            void init()  ;
            void bind()  ;
            bool start()  ;
            bool is_running() const  ;
            bool stop()  ;
            void send(char * data, size_t size , std::string client_id);
            boost::signals2::connection connect_on_data_received(std::function<void(const char * data, size_t size,std::string client_id)> func);

		private:
            std::string ipaddress_;
            int port_;
            boost::asio::ip::tcp::endpoint getEndPoint(std::string ipaddress, std::string port_number);
            void handle_read(const boost::system::error_code &erro, size_t bytes_transferred, std::shared_ptr<TCPClientEntity> client, std::string clientid);
			void handle_accept(std::shared_ptr<boost::asio::ip::tcp::socket> client, const boost::system::error_code& error);
			void workerThread(std::shared_ptr<boost::asio::io_service> ioService);
			void messagePackaged(const char * buffer, size_t size, std::string client_id);
            std::string get_new_client_id();

			std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
			std::shared_ptr<boost::asio::ip::tcp::socket> client_;
			std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
			std::shared_ptr<boost::asio::io_service::work> work_;
			std::shared_ptr<boost::asio::io_service> ioService_;

            boost::signals2::signal<void(std::string)> newclientconnected_;
            boost::signals2::signal<void(const char * data, size_t size,std::string client_id)> data_received_connections_;
			boost::asio::ip::tcp::endpoint serverEndPoint_;
			boost::thread_group threadGroup_; 
            bool isRunning_;
			std::array<char, 22808> receiveBuffer_; 
			std::mutex mtx;
			std::mutex mtx_r;
			std::mutex mtx_accept;
			std::mutex mtx_id;
            std::map<std::string, std::shared_ptr<TCPClientEntity>> connection_map_;
		};
 
