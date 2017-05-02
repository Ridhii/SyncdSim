#ifndef PROTOCOL_HANDLER_HPP
#define PROTOCOL_HANDLER_HPP

#include "common.hpp"
#include "context.hpp"
#include <queue>

class protocolHandler
{
private:
	Context* myContext;
	std::queue<


public:
	protocolHandler(Context* context);
	~protocolHandler();
	virtual void handleMemOpRequest() = 0;
	virtual void checkIncomingMsgQueue() = 0;

};

#endif