#include "MSIHandler.hpp"

MSIHandler::MSIHandler(Context* context) {
	myContext = context;
}



MSIHandler::~MSIHandler() {

}


// WHEN MOVING TO blockedMsgMap, what if some message has been blocked once before? 
// aka they should be inserted into front of queue instead of back of queue

void MSIHandler::handleMemOpRequest() {
	MemOp currOp = myContext -> getMemOp();
	uint64_t addr = currOp.addr;
	int myId = myContext -> getContextId();

	if (currOp.actionType = action_type.action_type_write) {
		/* 
		*  line not found in map -- INVALID 
		*  send a WRITE_MISS message to home node
		*  expect a DATA_VALUE_REPLY from home node
		*/
		if (cacheLineStatus.find(addr) == cacheLineStatus.end()) {
			Message* outMsg = new Message(
				myId, addr, MsgType.WRITE_MISS, nodeLatency);
			int homeNodeId = myContext -> getHomeNodeIdByAddr(addr);
			(context -> getContextById(homeNodeId)) -> addToIncomingMsgQueue(outMsg);
		}
		/* 
		*  line found in SHARED state
		*  send a INVALIDATE message to home node, requesting to invalidate other sharers
		*  expect a INVALIDATE_ACK from home node
		*/
		else if (cacheLineStatus[addr].MSIStatus == MSIStatus.SHARED) {
			Message* outMsg = new Message(
				myId, addr, MsgType.INVALIDATE, nodeLatency);
			int homeNodeId = myContext -> getHomeNodeIdByAddr(addr);
			(context -> getContextById(homeNodeId)) -> addToIncomingMsgQueue(outMsg);
		}
		/* 
		*  line found in MODIFIED state
		*  No need to contact home node
		*  send an CACHE_UPDATE to local cache
		*  expect a CACHE_UPDATE_ACK from home node
		*/
		else {
			Message* outMsg = new Message(
				myId, addr, MsgType.CACHE_UPDATE, cacheLatency);
			myContext -> addCacheMsg(outMsg);
		}
		return;
	}
	// if it's a READ request
	else {
		/* 
		*  line not found in map -- INVALID 
		*  send a READ_MISS message to home node
		*  expect a DATA_VALUE_REPLY from home node
		*/
		if (cacheLineStatus.find(addr) == cacheLineStatus.end()) {
			Message* outMsg = new Message(
				myId, addr, MsgType.READ_MISS, nodeLatency);
			int homeNodeId = myContext -> getHomeNodeIdByAddr(addr);
			(context -> getContextById(homeNodeId)) -> addToIncomingMsgQueue(outMsg);
		}
		/*
		* line found in either SHARED or MODIFIED state
		* send a CACHE_READ to local cache
		* expect a CACHE_READ_REPLY
		*/
		else {
			Message* outMsg = new Message(
				myId, addr, MsgType.CACHE_READ, cacheLatency);
			myContext -> addCacheMsg(outMsg);
		}

		return;	
	}

}

void sendMsgToNode(int dstId, uint64_t addr, MessageType msgType) {
	int myId = myContext -> getContextId();
	Message* outMsg = new Message(
	 					myId, addr, MessageType.DATA_VALUE_REPLY, nodeLatency);
	Context* dstContext = getContextById(dstId);
	dstContext -> addToIncomingMsgQueue(outMsg);
}


void sendMsgToCache(uint64_t addr, MessageType msgType) {
	int myId = myContext -> getContextId();
	Message* outMsg = new Message(
						myId, addr, MessageType.CACHE_INVALIDATE, cacheLatency);
	myContext -> addToCacheMsgQueue(outMsg);
}


void addToBlockedMsgMap(Message* msg) {
	uint64_t addr = msg -> addr;
	std::map<uint64_t, std::queue<Message*> > blockedMsgMap = myContext -> getBlockedMsgMap();
	if (blockedMsgMap.find(addr) != blockedMsgMap.end()) {
		// is the queue going to be updated in this way?
		blockedMsgMap[addr].emplace(msg);
	}
	else {
		std::queue<Message*> q;
		q.emplace(msg);
		blockedMsgMap.insert(std::pair<uint64_t, std::queue<Message*> > (addr, q));
	}
}



bool MSIHandler::handleMessage(Message* msg) {

	 	MessageType msgType = msg -> msgType;
	 	uint64_t addr = msg -> addr;
	 	int srcId = msg -> sourceID;

	 	switch (msgType) {
	 		//=============================== READ_MISS ===============================
	 		case READ_MISS:
	 			if (blockedMsgMap.find(addr) == blockedMsgMap.end()) { // not blocked
	 				DirectoryEntry entry = myContext -> lookupDirectoryEntry(addr);
	 				if (entry.status == DirectoryEntryStatus.SHARED || 
	 					entry.status == DirectoryEntryStatus.UNCACHED) {
	 					sendMsgToNode(srcId, addr, MessageType.DATA_VALUE_REPLY);	 					
	 					myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus.SHARED, srcId);
	 				}
	 				else {	// MODIFIED
	 					int ownerId = 0;
	 					for (bool isOwner : entry.processorMask) {
	 						if (isOwner)	break;
	 						ownerId++;
	 					}
	 					sendMsgToNode(ownerId, addr, MessageType.FETCH);
	 					return false;
	 				}
	 			}
	 			else { // line @ addr is blocked
	 				return false;
	 			}
	 			break;

	 		//=============================== WRITE_MISS ===============================
	 		case WRITE_MISS:
	 			if (blockedMsgMap.find(addr) == blockedMsgMap.end()) {
	 				DirectoryEntry entry = myContext -> lookupDirectoryEntry(addr);
	 				if (entry.status == DirectoryEntryStatus.UNCACHED) {
	 					sendMsgToNode(srcId, addr, MessageType.DATA_VALUE_REPLY);	 					
	 					myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus.MODIFIED, srcId);
	 				} 
	 				else if (entry.status == DirectoryEntryStatus.SHARED) {
	 					int sharerId = 0;
	 					int sharerCount = 0;
	 					for (bool isSharer : entry.processorMask) {
	 						if (isSharer) {
	 							sendMsgToNode(sharerId, addr, MessageType.INVALIDATE);
	 							sharerCount ++;
	 						}
	 						sharerId++;
	 					}
	 					pendingInvAckCount.insert(std::pair<uint64_t, int> (addr, sharerCount));
	 					return false;
	 				}
	 				else {	// MODIFIED
	 					int ownerId = 0;
	 					for (bool isOwner : entry.processorMask) {
	 						if (isOwner)	break;
	 						ownerId++;
	 					}
	 					sendMsgToNode(ownerId, addr, MessageType.FETCH_INVALIDATE);
	 					return false;
	 				}
	 			}
	 			else {
	 				return false;
	 			}
	 			break;

	 		//=============================== INVALIDATE ===============================
	 		case INVALIDATE:
	 			if (blockedMsgMap.find(addr) == blockedMsgMap.end()) {
	 				int homeId = myContext -> getHomeNodeIdByAddr(addr);
	 				int myId = myContext -> getContextId();
	 				/* if I am the home node, this request is for me to send out 
	 				 * further INVALIDATE requests to the sharers except for the sender. 
					*/
	 				if (homeId == myId) {
	 					// assert that the entry status is indeed in SHARED 
	 					DirectoryEntry entry = myContext -> lookupDirectoryEntry(addr);
	 					int sharerId = 0;
	 					int sharerCount = 0;
	 					for (bool isSharer : entry.processorMask) {
	 						if (isSharer && srcId != sharerId) {
	 							sendMsgToNode(sharerId, addr, MessageType.INVALIDATE);
	 							sharerCount ++;
	 						}
	 						sharerID++;
	 					}
	 					pendingInvAckCount.insert(std::pair<uint64_t, int> (addr, sharerCount));
	 					return false;
	 				}
	 				else {
	 					/*
		 				 * if I am not the home node, the request is for me to invalidate 
		 				 * my cache line and ACK
		 				 *	
		 				 * It's possible for the line to be INVALID in my own cache due to
		 				 * eviction. In that case, just send ACK right away
		 				 */
		 				if (cacheLineStatus.find(addr) == cacheLineStatus.end()) { // invalid
		 					sendMsgToNode(srcId, addr, MessageType.INVALIDATE_ACK);
		 				}
		 				else { 
		 				// needs to ask cache to invalidate the line
		 					sendMsgToCache(addr, MessageType.CACHE_INVALIDATE);
		 					return false;
		 				}
	 				}
	 			}
	 			else {
	 				
	 				return false;
	 			}
	 			break;

	 		//=============================== INVALIDATE_ACK ===============================
	 		case INVALIDATE_ACK:
	 			int homeId = myContext -> getHomeNodeIdByAddr(addr);
	 			int myId = myContext -> getContextId();
	 			/* 
	 			 * if I am the home node, this ACK is from a sharer of the line 
	 			 * decrement the count from pendingInvAckCount
	 			 * if reaches 0, 
	 			 * remove entry from pendingInvAckCount
	 			 * unblock the first message in blockedMap for this line and service it
	 			 * modify directory status accordingly
	 			 * move blocked queue to front of incoming queue and remove this map entry
				*/
	 			if (homeId == myId) {
	 				pendingInvAckCount[addr]--;
	 				if (pendingInvAckCount[addr] == 0) {
	 					pendingInvAckCount.erase[addr];
	 					Message* msg = blockedMsgMap[addr].top();
	 					blockedMsgMap[addr].pop();
	 					int srcId = msg -> sourceID;
	 					// assert - must be either WRITE_MISS or INVALIDATE
	 					if (msg -> msgType == msgType.WRITE_MISS) {
	 						sendMsgToNode(srcId, addr, MessageType.DATA_VALUE_REPLY);	 					
	 						myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus.MODIFIED, srcId);
	 					} 
	 					else if (msg -> msgType == msgType.INVALIDATE) {
	 						sendMsgToNode(srcId, addr, MessageType.INVALIDATE_ACK);	 					
	 						myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus.MODIFIED, srcId);
	 					}
	 					else {
	 						// ERROR!
	 					}
	 					while (!blockedMsgMap[addr].empty()) {
	 						Message* msg = blockedMsgMap[addr].top();
	 						if (handleMessage(msg)) { // not blocked
	 							blockedMsgMap[addr].pop();
	 							continue;
	 						}	
	 						else { // message blocked again, then the rest of the blocked queue remain
	 							break;
	 						}
	 					}	
	 				}
	 			}
	 			/* 
	 			 * if I am not the home node, this ACK must be from the home node 
	 			 * telling me that I can proceed to writing the line
	 			 * modify my cache and cacheLineStatus
	 			 * set successful to true so that processor can take the next memop request
				*/
	 			else {
	 				cacheLineStatus[addr] = MSIStatus.MODIFIED;
	 				sendMsgToCache(addr, Message.CACHE_UPDATE);
	 				myContext -> setSuccessful(true);
	 			}
	 			break;

	 		//=============================== FETCH ===============================
	 		case FETCH:
				cacheLineStatus[addr] = MSIStatus.SHARED;
	 			sendMsgToCache(addr, Message.CACHE_FETCH);
	 			break;

	 		//=============================== FETCH_INV ===============================
	 		case FETCH_INVALIDATE:
	 			cacheLineStatus[addr] = MSIStatus.INVALID;
	 			sendMsgToCache(addr, Message.CACHE_INVALIDATE);
	 			break;

	 		//=============================== DATA_VALUE_REPLY ===============================
	 		case DATA_VALUE_REPLY:
	 			// you must be the requesting node and successful must be 0 right now
		// send an UPDATE_LINE to cache and wait for UPDATE_LINE_ACK
		// satisfy the curr request and set successful = 1
		

	 			break;
	 		case DATA_WRITE_BACK:
	 			break;
	 		case CACHE_READ_REPLY:
	 			break;
	 		case CACHE_UPDATE_ACK:
	 			break;
	 		case CACHE_INVALIDATE_ACK:
	 			break;
	 		case CACHE_EVICTION_ALERT:
	 			break;
	 		default: // throw exception
	 			break;

	 	}
	 	return true;
}




void MSIHandler::checkIncomingMsgQueue() {
	std::queue<Message*> messages = myContext -> getIncomingMsgQueue();
	std::map<uint64_t, std::queue<Message*> > blockedMsgMap = myContext -> getBlockedMsgMap();

	/* 
	* First, loop through the entire incomingMsgQueue and all entries in blockedMsgMap
	* to decrement their latency count
	*/
	for (Message* msg : messages) {
		if (msg -> latency > 0) {
			msg -> latency--;
		}
	}
	for (auto itr = blockedMsgMap.begin(); itr != blockedMsgMap.end(); ++itr) {
		for (auto msg : itr -> second) {	// map values are queues
			if (msg -> latency > 0) {	
				msg -> latency--;
			}
		}
	}

	Message* msg;
	while (!messages.empty()) {
		msg = messages.top();
		messages.pop();
		if (!handleMessage(msg)){ // if message blocked, add to blocked map
			addToBlockedMsgMap(msg);
		}
	}
}
