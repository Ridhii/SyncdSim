#ifndef MSI_HANDLER_HPP
#define MSI_HANDLER_HPP

#include "protocolHandler.hpp"
#include "context.hpp"
#include "common.hpp"
#include <map>


enum MSIStatus {MODIFIED, SHARED, INVALID};


class MSIHandler: public protocolHandler {
private:
	// should match status of local cache
	std::map<uint64_t, MSIStatus> cacheLineStatus;

public:
	MSIHandler(Context* context);
	~MSIHandler();
	void handleMemOpRequest();
	void checkIncomingMsgQueue();
};


#endif