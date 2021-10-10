
#include <thread>

#include "CommonFunction.h"
#include "tcpserver.h"
#include "tundevicecommunication.h"

std::shared_ptr<TCPServer> server_;
std::shared_ptr<TunDeviceCommunication> dev_;

std::string client_id_ = "";
void client_connected(std::string ip, std::string port, std::string client_id)
{
    printf("remote ipaddress is %s  connected new id is: %s\n",  ip.c_str(), client_id.c_str());
    client_id_ = client_id;
}

void tcp_data_received(const char * data, size_t size, std::string client_id)
{
    client_id_ = client_id;
    printf("data received to main\n ");
    std::string temp(data, data+size);
    dev_->send(data, size);
    printf("data is: %s\n", temp.c_str());
}
void tun_data_received(char * data, size_t size)
{
    if(server_ != nullptr && server_->is_running() && client_id_.size() >1)
    {
        server_->send(data, size, client_id_);
        printf("send %d bytes to client \n", size);
    }
    else
    {
        printf("there is no active session\n");
    }
}
int main(int argc, char * argv[])
{
    if(argc < 5)
    {
        printf("please enter arguments:" );
        printf("-t tun\\tap device name");
        printf("-b server binding properties ip:port for example 0.0.0.0:2343\n" );
        return -1;
    }

    int index  = 1;
    std::string tun_name = "";
    std::string ip = "";
    int port;
    while(index < argc)
    {
        std::string param = argv[index];
        if(param == "-t")
        {
            tun_name = argv[++index];
            dev_ = std::make_shared<TunDeviceCommunication>(tun_name);
            dev_->connect_receive(std::bind(tun_data_received, std::placeholders::_1, std::placeholders::_2));
            dev_->start();
        }
        if(param == "-b")
        {
            ++index;
            std::vector<char>  binding_properties(argv[index],argv[index] +  strlen(argv[index]));
            std::vector<char> splitter{':'};
            std::vector<std::vector<char>> binding_param =CommonFunction::split(binding_properties, splitter);
            if(binding_param.size() == 2)
            {
                std::string temp1(binding_param[0].data(), binding_param[0].size());
                std::string temp2(binding_param[1].data(), binding_param[1].size());
                ip = temp1;
                port = std::atoi(temp2.c_str());
                printf("binding to : %s with port number  %d \n", ip.c_str(), port);
            }
            else
            {
                return 0;
            }
        }
        index++;
    }
    if(tun_name.size() ==0)
        return -1;
    if(dev_ != nullptr)
    {
        server_ =std::make_shared<TCPServer>(ip, port);
        server_->connect_on_data_received(std::bind(tcp_data_received, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        server_->connect_on_client_connected(std::bind(client_connected, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        server_->init();
        server_->bind();
        server_->start();
        while(true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
    else
    {
        printf("please enter tun device name");
    }
    return 0;
}
