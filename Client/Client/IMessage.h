#pragma once


class IMessage {

public :
	enum MessageType {
		ConnectionRequest,
	};
	virtual MessageType get_type() const = 0;
};