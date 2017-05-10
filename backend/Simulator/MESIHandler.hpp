#ifndef MESI_HANDLER_HPP
#define MESI_HANDLER_HPP

#include "context.hpp"
class Context;

class MESIHandler: public ProtocolHandler {
private:
	Context* myContext;
	// should match status of local cache
	std::map<uint64_t, protocolStatus> cacheLineStatus;
	// as home node, how many more INVALIDATE_ACK we need 
	std::map<uint64_t, int> pendingInvAckCount;

	std::map<uint64_t, std::vector<Message*> > blockedMsgMap;

public:
	MESIHandler(Context* context);
	~MESIHandler();
	void handleMemOpRequest();
	void checkIncomingMsgQueue();
	bool handleMessage(Message* msg);

	void sendMsgToNode(int dstId, uint64_t addr, MessageType MessageType);
	void sendMsgToCache(uint64_t addr, MessageType MessageType);
	void addToBlockedMsgMap(Message* msg);
	void checkBlockedQueueAtAddress(uint64_t addr);
};


#endif