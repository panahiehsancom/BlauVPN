#pragma once

#include <string>
#include <vector>

#include "ISerializableMessage.h"

class ConnectionRequestMessage : public ISerializableMessage
{
public:
	// Inherited via ISerializableMessage
	MessageType get_type() const override;
	void serialize(char * output, size_t size) override;
	void deserialize(const char * data, size_t size) override;
	enum ProtocolType {
		TCP,
		UDP,
		ICMP
	};

private:
	ProtocolType protocol_type_;
	std::string ipaddress_;
	std::string port_number;
	std::vector<char> data_gram_;
};

