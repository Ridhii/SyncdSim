#ifndef PROTOCOL_HANDLER_HPP
#define PROTOCOL_HANDLER_HPP

#include "common.hpp"
#include "context.hpp"
#include "message.hpp"
#include <queue>

class protocolHandler
{
private:
	Context* myContext;

public:
	protocolHandler(Context* context);
	~protocolHandler();
	virtual void handleMemOpRequest() = 0;
	virtual void checkIncomingMsgQueue() = 0;

};

#endif