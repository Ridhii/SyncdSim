#ifndef PROTOCOL_HANDLER_HPP
#define PROTOCOL_HANDLER_HPP

#include "common.hpp"



class Context; 
class ProtocolHandler
{
private:
	Context* myContext;

public:
	ProtocolHandler(Context* context);
	~ProtocolHandler();
	virtual void handleMemOpRequest() = 0;
	virtual void checkIncomingMsgQueue() = 0;

};

#endif