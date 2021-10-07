#pragma once

#include "IMessage.h"
class ISerializableMessage : public IMessage {

public:
	 
	virtual void serialize(char * output, size_t size) = 0;
	virtual void deserialize(const char * data, size_t size) = 0;

};