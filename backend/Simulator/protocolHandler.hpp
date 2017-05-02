#ifndef PROTOCOL_HANDLER_HPP
#define PROTOCOL_HANDLER_HPP

#include "common.hpp"
#include "context.hpp"

class protocolHandler
{
private:
	Context* myContext;


public:
	protocolHandler(Context* context);
	~protocolHandler();
	virtual void handleMemOpRequest() = 0;


};

#endif