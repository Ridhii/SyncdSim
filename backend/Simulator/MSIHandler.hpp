#ifndef MSI_HANDLER_HPP
#define MSI_HANDLER_HPP

#include "protocolHandler.hpp"

class MSIHandler: public protocolHandler {
public:
	MSIHandler(Context* context);
	~MSIHandler();
	void handleMemOpRequest();
	void checkIncomingMsgQueue();
};


#endif