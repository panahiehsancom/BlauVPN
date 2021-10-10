#include "ConnectionRequestMessage.h"

IMessage::MessageType ConnectionRequestMessage::get_type() const
{
	return MessageType::ConnectionRequest;
}

void ConnectionRequestMessage::serialize(char * output, size_t size)
{

}

void ConnectionRequestMessage::deserialize(const char * data, size_t size)
{
	int index = 0;
	char type = data[0];
	index++;
	//data_gram_.

}
