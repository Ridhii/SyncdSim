#include "MSIHandler.hpp"


MSIHandler::MSIHandler(Context* context) {
	myContext = context;
}

MSIHandler::~MSIHandler() {

}

void MSIHandler::sendMsgToNode(int dstId, uint64_t addr, MessageType MessageType) {
	
	myContext->incNumSentMsgs();
	int myId = myContext -> getContextId();
	Message* outMsg = new Message(
	 					myId, addr, MessageType, nodeLatency);
	Context* dstContext = myContext -> getContextById(dstId);
	dstContext -> addToIncomingMsgQueue(outMsg);
}


void MSIHandler::sendMsgToCache(uint64_t addr, MessageType MessageType) {

	myContext->incNumSentMsgsToCache();
	int myId = myContext -> getContextId();
	Message* outMsg = new Message(
						myId, addr, MessageType, cacheLatency);
	myContext -> addToCacheMsgQueue(outMsg);
}


void MSIHandler::addToBlockedMsgMap(Message* msg) {
	uint64_t addr = msg -> addr;
	if (blockedMsgMap.find(addr) != blockedMsgMap.end()) {
		// is the queue going to be updated in this way?
		if (msg->msgType == MessageType::FETCH || 
			msg->msgType == MessageType::FETCH_INVALIDATE ||
			msg->msgType == MessageType::INVALIDATE) {
			blockedMsgMap[addr].insert(blockedMsgMap[addr].begin(), msg);
		}
		else {
			blockedMsgMap[addr].push_back(msg);
		}
	}
	else {
		std::vector<Message*> q;
		q.push_back(msg);
		blockedMsgMap.insert(std::pair<uint64_t, std::vector<Message*> > (addr, q));
	}
}

void MSIHandler::checkBlockedQueueAtAddress(uint64_t addr) {
	
	Message* m;
	while (!blockedMsgMap[addr].empty()) {
	 	m = blockedMsgMap[addr].front();
	 	/* if a message is serviced, then don't service it again */
	 	if(m->serviced){
	 		break;
	 	}
	 	if (handleMessage(m, true)) { // not blocked
	 		blockedMsgMap[addr].erase(blockedMsgMap[addr].begin());
	 	}	
	 	else { // message blocked again, rest of the queue remain blocked
	 		break;
	 	}
	}	
	
	// if queue becomes empty, remove it from map
	if (blockedMsgMap[addr].empty()) {
	 	blockedMsgMap.erase(addr);
	}
}


void MSIHandler::handleMemOpRequest() {
	
	MemOp currOp = myContext -> getMemOp();
	uint64_t addr = currOp.addr;
	int myId = myContext -> getContextId();
    
    //cout << "********  NEW OP REQUEST  FOR CONTEXT " << myContext->getContextId() << " ********\n";
    //cout << "memory action is " << currOp.actionType << " and" << std::hex << " addr is " << addr << "in context " << myContext->getContextId() << "\n";
	if (currOp.actionType == contech::action_type::action_type_mem_write) {
		/* 
		*  line not found in map -- INVALID 
		*  send a WRITE_MISS message to home node
		*  expect a DATA_VALUE_REPLY from home node
		*/
		if (cacheLineStatus.find(addr) == cacheLineStatus.end()) {
		    myContext->incCacheMiss();			
			int homeNodeId = myContext -> getHomeNodeIdByAddr(addr);
			//cout << "line in an invalid state, sending a WRITE_MISS to homeNode " << homeNodeId << "\n";
			sendMsgToNode(homeNodeId, addr, MessageType::WRITE_MISS);
		}
		/* 
		*  line found in SHARED state
		*  send a INVALIDATE message to home node, requesting to invalidate other sharers
		*  expect a INVALIDATE_OTHER_ACK from home node
		*/
		else if (cacheLineStatus[addr] == protocolStatus::S) {
			myContext->incCacheHit();
			myContext->incNumInvalidations();
			int homeNodeId = myContext -> getHomeNodeIdByAddr(addr);
			//cout << "line in a shared state, sending an INVALIDATE_OTHER to homeNode " << homeNodeId << "\n";
			sendMsgToNode(homeNodeId, addr, MessageType::INVALIDATE_OTHER);
		}
		/* 
		*  line found in MODIFIED state
		*  No need to contact home node
		*  send a CACHE_UPDATE to local cache
		*  expect a CACHE_UPDATE_ACK from home node
		*/
		else {
			myContext->incCacheHit();
			//cout << "line in a MODIFIED state already!\n" ;
			//cout << "sending cache_update for address " << addr << endl;
			Message* outMsg = new Message(myId, addr, MessageType::CACHE_UPDATE, cacheLatency);
			myContext -> addToCacheMsgQueue(outMsg);
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
			myContext->incCacheMiss();	
			int homeNodeId = myContext -> getHomeNodeIdByAddr(addr);
			sendMsgToNode(homeNodeId, addr, MessageType::READ_MISS);
		}
		/*
		* line found in either SHARED or MODIFIED state
		* send a CACHE_READ to local cache
		* expect a CACHE_READ_REPLY
		*/
		else {
			//cout << "READ HIT for context " << myContext->getContextId() << "\n";
			myContext->incCacheHit();
			Message* outMsg = new Message(
				myId, addr, MessageType::CACHE_READ, cacheLatency);
			myContext -> addToCacheMsgQueue(outMsg);
		}

		return;	
	}

}



bool MSIHandler::handleMessage(Message* msg, bool blocked) {

	 	MessageType type = msg -> msgType;
	 	uint64_t addr = msg -> addr;
	 	int srcId = msg -> sourceID;
	 	assert(msg->serviced == false);


	 	//cout << "context " << myContext->getContextId() << " recvd a msg " << mString[type] << " from node " << srcId << " for addr " << std::hex << addr << "\n";
	 	MemOp currOp = myContext -> getMemOp();
	 	//printf("currOp action is %d  and addr is %llx \n", currOp.actionType, currOp.addr);
	 	uint64_t opAddr = currOp.addr;

	 	Message* m;
	 	
	 	int homeId = myContext -> getHomeNodeIdByAddr(addr);
	 	int myId = myContext -> getContextId();

	 	switch (type) {
	 		//=============================== READ_MISS ===============================
	 		case READ_MISS:
	 			if (blockedMsgMap.find(addr) == blockedMsgMap.end() || blocked) { // not blocked

	 				DirectoryEntry entry = myContext -> lookupDirectoryEntry(addr);
	 			    msg->serviced = true;
	 				if (entry.status == DirectoryEntryStatus::SHARED || 
	 					entry.status == DirectoryEntryStatus::UNCACHED) {
	 					sendMsgToNode(srcId, addr, MessageType::DATA_VALUE_REPLY);	 					
	 					myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus::SHARED, srcId);
	 				}
	 				else {	// MODIFIED

	 					int ownerId = 0;
	 					for (bool isOwner : entry.processorMask) {
	 						if (isOwner)	break;
	 						ownerId++;
	 					}
	 					//cout << "recvd a READ_MISS for addr " << addr << "for a line in modified state, sending a FETCH to ownerId " << ownerId << "\n"; 
	 					sendMsgToNode(ownerId, addr, MessageType::FETCH);
	 					return false;
	 				}
	 			}
	 			else { // line @ addr is blocked
	 				return false;
	 			}
	 			break;

	 		//=============================== WRITE_MISS ===============================
	 		case WRITE_MISS:
	 			if (blockedMsgMap.find(addr) == blockedMsgMap.end() || blocked) {
	 				msg->serviced = true;
	 				//cout << "there is no entry for this address in blockedMsgMap \n" ;
	 				DirectoryEntry& entry = myContext -> lookupDirectoryEntry(addr);
	 				if (entry.status == DirectoryEntryStatus::UNCACHED) {
	 					//cout << "sending a DATA_VALUE_REPLY to node" << srcId << "\n";
	 					sendMsgToNode(srcId, addr, MessageType::DATA_VALUE_REPLY);	 					
	 					myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus::MODIFIED, srcId);
	 				} 
	 				else if (entry.status == DirectoryEntryStatus::SHARED) {
	 					int sharerId = 0;
	 					int sharerCount = 0;
	 					for (int isSharer : entry.processorMask) {
	 						if (isSharer) {
	 							sendMsgToNode(sharerId, addr, MessageType::INVALIDATE);
	 							sharerCount ++;
	 						}
	 						sharerId++;
	 					}
	 					pendingInvAckCount.insert(std::pair<uint64_t, int> (addr, sharerCount));
	 					return false;
	 				}
	 				else {	// MODIFIED
	 					//printf("directory entry is already MODIFIED\n");
	 					int ownerId = 0;
	 					for (int isOwner : entry.processorMask) {
	 						if (isOwner)	break;
	 						ownerId++;
	 					}
	 					assert(ownerId < myContext->getNumContexts());
	 					//cout << "sending a FETCH_INVALIDATE to ownerId " << ownerId << "\n";
	 				    sendMsgToNode(ownerId, addr, MessageType::FETCH_INVALIDATE);
	 					return false;
	 				}
	 			}
	 			else {
	 				return false;
	 			}
	 			break;

	 		//=============================== INVALIDATE ===============================
	 		case INVALIDATE:
	 				/*the request is for me to invalidate 
		 			* my cache line and ACK
		 			*	
		 			* It's possible for the line to be INVALID in my own cache due to
		 			* eviction. In that case, just send ACK right away
		 			*/
		 			if (cacheLineStatus.find(addr) == cacheLineStatus.end()) { // invalid
		 				//cout << " The line is not in my cache due to eviction, sending an INVALIDATE_ACK right away \n";
		 				sendMsgToNode(srcId, addr, MessageType::INVALIDATE_ACK);
		 			}
		 			else { 
		 				/* needs to ask cache to invalidate the line and 
		 				 * remove the entry from cacheLineStatus to indicate an INVALID status
		 				 */
		 				//cout << "sending a CACHE_INVALIDATE to cache and removing the entry from cacheLineStatus\n";
		 				msg->serviced = true;
		 				cacheLineStatus.erase(addr);
		 				sendMsgToCache(addr, MessageType::CACHE_INVALIDATE);
		 				return false;
		 			}
		 		
		 		break;


		 	//=============================== INVALIDATE_OTHER ===============================
		 	case INVALIDATE_OTHER:
		 		assert(homeId == myId);
		 		/* I am the home node, this request is for me to send out 
	 			*  further INVALIDATE requests to the sharers except for the sender. 
				*/
	 			if (blockedMsgMap.find(addr) == blockedMsgMap.end() || blocked) {
	 				msg->serviced = true;
	 				DirectoryEntry entry = myContext -> lookupDirectoryEntry(addr);
	 				if(entry.status != DirectoryEntryStatus::SHARED){
	 					
	 					 /* cannot service this INVALIDATE_OTHER as the directory entry is already
	 					    in modified state and hence somebody else is the owner of this
	 					    line
	 					 */
	 					//cout << "context" << myContext->getContextId() << "sending a ROLLBACK to node " << srcId << "\n";

	 					sendMsgToNode(srcId, addr, MessageType::ROLLBACK);
	 					return true;
	 				}
	 				assert(entry.status == DirectoryEntryStatus::SHARED);
	 				int sharerId = 0;
	 				int sharerCount = 0;
	 				for (bool isSharer : entry.processorMask) {
	 					if (isSharer && srcId != sharerId) {
	 						//cout << "sending INVALIDATE to sharer " << sharerId << "\n";
	 						sendMsgToNode(sharerId, addr, MessageType::INVALIDATE);
	 						sharerCount ++;
	 					}
	 					sharerId++;
	 				}
	 				if(sharerCount > 0){
	 					pendingInvAckCount.insert(std::pair<uint64_t, int> (addr, sharerCount));
	 					return false;
	 				}
	 				else{
	 				    /* There is only one sharer that is the requester, reply to it immediately */
	 				    myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus::MODIFIED, srcId);
	 				    sendMsgToNode(srcId, addr, MessageType::INVALIDATE_OTHER_ACK);	 				  
	 				}
	 			}
	 			else {
	 				return false;
	 			}
	 			break;


	 		//=============================== INVALIDATE_ACK ===============================
	 		case INVALIDATE_ACK:
	 			
	 			/* 
	 			 * I am the home node, this ACK is from a sharer of the line 
	 			 * decrement the count from pendingInvAckCount
	 			 * if reaches 0, 
	 			 * remove entry from pendingInvAckCount
	 			 * modify directory status accordingly (by making the original requester an exclusive)
	 			 * service the first message from the blocked queue
	 			 * try to go down the queue until blocked again
				*/
	 			assert(homeId == myId);
	 			pendingInvAckCount[addr]--;
	 			if (pendingInvAckCount[addr] == 0) {
	 				pendingInvAckCount.erase(addr);
	 				Message* m = blockedMsgMap[addr].front();
	 				blockedMsgMap[addr].erase(blockedMsgMap[addr].begin());
	 				int blockedSrcId = m -> sourceID;
	 				myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus::MODIFIED, blockedSrcId);

	 				// assert - must be either WRITE_MISS or INVALIDATE_OTHER
	 				assert(m->msgType == MessageType::WRITE_MISS || m->msgType == MessageType::INVALIDATE_OTHER);
	 				if (m -> msgType == MessageType::WRITE_MISS) {
	 					//cout << "sending out a DATA_VALUE_REPLY to node " << srcId << "upon receiving all INVALIDATE_ACKS \n";
	 					sendMsgToNode(blockedSrcId, addr, MessageType::DATA_VALUE_REPLY);	 					
	 				} 
	 				else if (m -> msgType == MessageType::INVALIDATE_OTHER) {
	 					//cout << "sending out an INVALIDATE_OTHER to node " << srcId << "upon receiving all INVALIDATE_ACKS \n";
	 					sendMsgToNode(blockedSrcId, addr, MessageType::INVALIDATE_OTHER_ACK);	 					
	 				}
	 				checkBlockedQueueAtAddress(addr);
	 				
	 			}
	 			break;

	 		//=============================== INVALIDATE_OTHER_ACK ===============================
	 		case INVALIDATE_OTHER_ACK:
	 			/* 
	 			 * this ACK must be from the home node 
	 			 * telling me that processor can proceed to writing the line
	 			 * modify cacheLineStatus and send cache an UPDATE message 
	 			 * 
	 			 * Note that we'll wait for CACHE_UPDATE_ACK to set successful to true
				*/
	 				//cout << "received an INVALIDATE_OTHER_ACK for my curr request\n";
	 			assert(srcId == homeId);
	 			assert(opAddr == addr);
	 			cacheLineStatus[addr] = protocolStatus::M;
	 			sendMsgToCache(addr, MessageType::CACHE_UPDATE);
	 			//cout << "sending cache_update for address " << addr << endl;

	 			break;

	 		//=============================== FETCH ===============================
	 		case FETCH:
	 		    //cout << "recvd a FETCH for addr " << addr << "on contextID " << myContext->getContextId() << "\n";
	 		    if(cacheLineStatus.find(addr) == cacheLineStatus.end()){
	 		    	/* we must have already sent out a DATA_WRITE_BACK due to eviction */
	 		    	return true;
	 		    }
	 		    //cout << "line status is currently: " << cacheLineStatus[addr] << endl;
	 		    assert(cacheLineStatus[addr] == protocolStatus::M);
				cacheLineStatus[addr] = protocolStatus::S;
	 			sendMsgToCache(addr, MessageType::CACHE_FETCH);
	 			msg->serviced = true;
	 			return false;
	 			break;

	 		//=============================== FETCH_INV ===============================
	 		case FETCH_INVALIDATE:
	 		    if(cacheLineStatus.find(addr) == cacheLineStatus.end()){
	 		    	/* we must have already sent out a DATA_WRITE_BACK due to eviction */
	 		    	return true;
	 		    }
	 		    assert(cacheLineStatus[addr] == protocolStatus::M);
	 			cacheLineStatus.erase(addr);
	 			sendMsgToCache(addr, MessageType::CACHE_INVALIDATE);
	 			msg->serviced = true;
	 			return false;
	 			break;

	 		//=============================== DATA_VALUE_REPLY ===============================
	 		case DATA_VALUE_REPLY:
	 			/*
	 			 * This must be a response for a previous message (either READ_MISS or 
	 			 * WRITE_MISS) we send out to home node
	 			 * change cacheLineStatus[addr]
	 			 * tell local cache to update the line
				*/
	 			assert(myContext->getSuccessful() == false);
	 			// if a WRITE_MISS
				if (currOp.actionType == contech::action_type::action_type_mem_write) {
					cacheLineStatus.insert(std::pair<uint64_t, protocolStatus> (addr,protocolStatus::M));
				}
				else { // if a READ_MISS
					cacheLineStatus.insert(std::pair<uint64_t, protocolStatus> (addr,protocolStatus::S));
				}
				//cout << "recvd a DATA_VALUE_REPLY, update the cache for address" << addr << endl;
				sendMsgToCache(addr, MessageType::CACHE_UPDATE);
	 			break;

	 		//=============================== DATA_WRITE_BACK ===============================
	 		case DATA_WRITE_BACK:
	 			/*
				* We are the home node of the line
				* This must be from exclusive owner, either replying to a FETCH/FETCH_INVALIDATE
				* we sent earlier, or notifying me about eviction
				*
				* Either way, we update directory entry and check the blocked queue for the line
	 			*/
	 			if (blockedMsgMap.find(addr) != blockedMsgMap.end()) {
	 				m = blockedMsgMap[addr].front();
	 				blockedMsgMap[addr].erase(blockedMsgMap[addr].begin());
	 				assert(m->msgType == MessageType::WRITE_MISS || m->msgType == READ_MISS);
	 				if (m -> msgType == MessageType::WRITE_MISS) {
	 					sendMsgToNode(m -> sourceID, addr, MessageType::DATA_VALUE_REPLY);
	 					myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus::MODIFIED, m -> sourceID);
	 				} 
	 				else if (m -> msgType == MessageType::READ_MISS) {
	 					sendMsgToNode(m -> sourceID, addr, MessageType::DATA_VALUE_REPLY);	
	 		 			myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus::SHARED, m -> sourceID); 					
	 				}
	 				checkBlockedQueueAtAddress(addr);
	 			}
	 			else{
	 				/* if we get a DATA_WRITE_BACK without a READ_MISS or WRITE_MISS, this means 
	 				   that the owner of this line must have evicted it.
	 				*/
	 				myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus::UNCACHED, srcId);
	 			}
	 			break;


	 		//=============================== CACHE_READ_REPLY ===============================	
	 		case CACHE_READ_REPLY:
	 			/* Must be a reply to my own memOp, which is a action_type_read 
	 			 * set successful to 1 so that processor can proceed to next mem op
	 			 */
	 			myContext -> setSuccessful(true);
	 			break;

	 		//=============================== CACHE_UPDATE_ACK ===============================	
	 		case CACHE_UPDATE_ACK:
	 			/*
	 			* Indicates a successful update
	 			* The address must match the addr of current mem op
	 			* we can now set successful to true so that processor can proceed
	 			*
	 			* Note that cacheLineStatus is already updated
	 			*/

	 			assert(opAddr == addr);
	 			//cout << "received a CACHE_UPDATE_ACK for current request\n";
	 			myContext -> setSuccessful(true);
	 			break;

	 		//=============================== CACHE_INVALIDATE_ACK ===============================	
	 		case CACHE_INVALIDATE_ACK:
	 			/*  
	 			* The first message of the blocked queue must be either an 
	 			* INVALIDATE or FETCH_INVALIDATE. We can now reply to it.
	 			* 
	 			* Note that cacheLineStatus is already updated
	 			*/

	 			m = blockedMsgMap[addr].front();
	 			blockedMsgMap[addr].erase(blockedMsgMap[addr].begin());

	 			assert(m->msgType == MessageType::INVALIDATE || m->msgType == MessageType::FETCH_INVALIDATE);
	 			if (m -> msgType == MessageType::INVALIDATE) {
	 				//cout << "recvd a CACHE_INVALIDATE_ACK, sending INVALIDATE_ACK to node " << m->sourceID << "\n";
	 				sendMsgToNode(m -> sourceID, addr, MessageType::INVALIDATE_ACK);	 					
	 			} 
	 			else if (m -> msgType == MessageType::FETCH_INVALIDATE) {
	 			    //cout << "recvd a CACHE_INVALIDATE_ACK, sending DATA_WRITE_BACK to node " << m->sourceID << "\n";
	 				sendMsgToNode(m -> sourceID, addr, MessageType::DATA_WRITE_BACK);	 					
	 			}
	 			checkBlockedQueueAtAddress(addr);
	 			break;

	 		//=============================== CACHE_FETCH_ACK ===============================	
	 		case CACHE_FETCH_ACK:
	 			/* 
	 			 * The first blocked message must be a FETCH. We can now reply to it.
	 			 *
	 			 * Note taht cacheLineStatus is already updated 
	 			 */
	 			m = blockedMsgMap[addr].front();
	 			blockedMsgMap[addr].erase(blockedMsgMap[addr].begin());

	 			// assert - must be FETCH
	 			//cout << "msgType is " << mString[m->msgType] << "and addr is " << m->addr << " and servicing node is " << myContext->getContextId() << "and sourceNode is " << m->sourceID << "\n";
	 			assert(m->msgType == MessageType::FETCH);
	 			sendMsgToNode(m -> sourceID, addr, MessageType::DATA_WRITE_BACK);	 
	 			checkBlockedQueueAtAddress(addr);					
	 			break;

	 		//=============================== CACHE_EVICTION_ALERT ===============================	
	 		case CACHE_EVICTION_ALERT:
	 			/*
	 			* if cacheLineStatus was MODIFIED, need to notify the home node
	 			* change cacheLineStatus to be INVALID by removing the entry
	 			*/
	 			if (cacheLineStatus[addr] == protocolStatus::M) {
					int homeNodeId = myContext -> getHomeNodeIdByAddr(addr);
					sendMsgToNode(homeNodeId, addr, MessageType::DATA_WRITE_BACK);
	 			}
	 			cacheLineStatus.erase(addr);
	 			break;
            //=============================== ROLLBACK ==============================================
	 		case ROLLBACK:
 				//cout << "contextId " << myContext->getContextId() << " received a ROLLBACK from node " << srcId << "\n";
 				myContext->reAddCurrMemOp();
 				myContext->setSuccessful(true);
 				break;


	 		default: // UNRECOGNIZED MESSAGE TYPE
	 			break;

	 	}
	 	return true;
}




void MSIHandler::checkIncomingMsgQueue() {
	std::vector<Message*>& messages = myContext -> getIncomingMsgQueue();
	//cout << "pH incoming message queue size: " << messages.size() << endl;
	/* 
	* First, loop through the entire incomingMsgQueue and all entries in blockedMsgMap
	* to decrement their latency count
	*/
	for (Message* msg : messages) {
		if (msg -> latency > 0) {
			msg -> latency--;
		}
	}
	int i;
	for(i = 0; i < messages.size(); i++){
		Message* msg = messages[i];
	    if (msg -> latency == 0) {
	    	if (!handleMessage(msg,false)) {
	    		//printf("adding to blockedMsgMap\n");
				addToBlockedMsgMap(msg);
			}
		}
		else{
			break;
		}
	}
	messages.erase(messages.begin(), messages.begin() + i);
}
