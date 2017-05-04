#ifndef MSI_HANDLER_HPP
#define MSI_HANDLER_HPP

#include "protocolHandler.hpp"
#include "context.hpp"
#include "common.hpp"
#include "directory.hpp"
#include <map>


enum MSIStatus {MODIFIED, SHARED, INVALID};


class MSIHandler: public protocolHandler {
private:
	// should match status of local cache
	std::map<uint64_t, MSIStatus> cacheLineStatus;
	// as home node, how many more INVALIDATE_ACK we need 
	std::map<uint64_t, int> pendingInvAckCount;

public:
	MSIHandler(Context* context);
	~MSIHandler();
	void handleMemOpRequest();
	void checkIncomingMsgQueue();
	void handleMessage(Message* msg);
};


#endif