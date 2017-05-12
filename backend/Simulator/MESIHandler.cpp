#include "MESIHandler.hpp"

MESIHandler::MESIHandler(Context* context) {
	myContext = context;
}

MESIHandler::~MESIHandler() {

}


void MESIHandler::sendMsgToNode(int dstId, uint64_t addr, MessageType MessageType) {
	
	myContext->incNumSentMsgs();
	int myId = myContext -> getContextId();
	Message* outMsg = new Message(
	 					myId, addr, MessageType, nodeLatency);
	Context* dstContext = myContext -> getContextById(dstId);
	dstContext -> addToIncomingMsgQueue(outMsg);
}


void MESIHandler::sendMsgToCache(uint64_t addr, MessageType MessageType) {

	myContext->incNumSentMsgsToCache();
	int myId = myContext -> getContextId();
	Message* outMsg = new Message(
						myId, addr, MessageType, cacheLatency);
	myContext -> addToCacheMsgQueue(outMsg);
	//cout << "messeging cache for line " << addr << "with type " << MessageType << endl;

}


void MESIHandler::addToBlockedMsgMap(Message* msg) {
	
	uint64_t addr = msg -> addr;
	if (blockedMsgMap.find(addr) != blockedMsgMap.end()) {
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

void MESIHandler::checkBlockedQueueAtAddress(uint64_t addr) {
	
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


void MESIHandler::handleMemOpRequest() {
	MemOp currOp = myContext -> getMemOp();
	uint64_t addr = currOp.addr;

	// cout << "********  NEW OP REQUEST  FOR CONTEXT " << myContext->getContextId() << " ********\n";
 //    cout << "memory action is " << currOp.actionType << " and" << std::hex << " addr is " << addr << "in context " << myContext->getContextId() << "\n";
	
	int myId = myContext -> getContextId();

	if (currOp.actionType == contech::action_type::action_type_mem_write) {
		/* 
		*  line not found in map -- INVALID 
		*  send a WRITE_MISS message to home node
		*  expect: DATA_VALUE_REPLY from home node
		*/
		if (cacheLineStatus.find(addr) == cacheLineStatus.end()) {
		    myContext->incCacheMiss();			
			int homeNodeId = myContext -> getHomeNodeIdByAddr(addr);
			// cout << "line in an invalid state, sending a WRITE_MISS to homeNode " << homeNodeId << "\n";
			sendMsgToNode(homeNodeId, addr, MessageType::WRITE_MISS);
		}
		/* 
		*  line found in SHARED state, we know that we're not the exclusive owner
		*  send a INVALIDATE message to home node, requesting to invalidate other sharers
		*  expect a INVALIDATE_ACK from home node
		*/
		else if (cacheLineStatus[addr] == protocolStatus::S) {
			myContext->incCacheHit();
			myContext->incNumInvalidations();
			int homeNodeId = myContext -> getHomeNodeIdByAddr(addr);
			// cout << "line in a shared state, sending an INVALIDATE to homeNode " << homeNodeId << "\n";
			sendMsgToNode(homeNodeId, addr, MessageType::INVALIDATE_OTHER);
		}
		/* 
		*  line found in MODIFIED state
		*  No need to contact home node
		*  send a CACHE_UPDATE to local cache
		*  expect a CACHE_UPDATE_ACK from cache
		*/
		else if (cacheLineStatus[addr] == protocolStatus::M){
			myContext->incCacheHit();
			// cout << "line in a MODIFIED state already!\n" ;
			Message* outMsg = new Message(myId, addr, MessageType::CACHE_UPDATE, cacheLatency);
			myContext -> addToCacheMsgQueue(outMsg);
			//cout << "messeging cache for line " << addr << "with type update" << endl;
		}
		/* 
		*  line found in EXCLUSIVE state
		*  No need to contact home node at this point
		*  send a CACHE_UPDATE to local cache
		*  expect a CACHE_UPDATE_ACK from cache
		*  promote cacheLineStatus to be MODIFIED
		*/

		else {
			//cout << "saving invalidation message" << endl;
			myContext->incEStateCount();
			assert(cacheLineStatus[addr] == protocolStatus::E); // must be in E state
			myContext->incCacheHit();
			// cout << "line in EXCLUSIVE state, promoting to MODIFIED!\n";
			Message* outMsg = new Message(myId, addr, MessageType::CACHE_UPDATE, cacheLatency);
			myContext -> addToCacheMsgQueue(outMsg);
			cacheLineStatus[addr] = protocolStatus::M;
			//cout << "messeging cache for line " << addr << "with type update" << endl;

		}
		return;
	}
	// if it's a READ request
	else {
		/* 
		*  line not found in map -- INVALID 
		*  send a READ_MISS message to home node
		*  expect either a DATA_VALUE_REPLY or a
		*  DATA_VALUE_REPLY_E (exclusive) from home node
		*/
		if (cacheLineStatus.find(addr) == cacheLineStatus.end()) {
			myContext->incCacheMiss();	
			int homeNodeId = myContext -> getHomeNodeIdByAddr(addr);
			sendMsgToNode(homeNodeId, addr, MessageType::READ_MISS);
		}
		/*
		* line found in SHARED, EXCLUSIVE, or MODIFIED state
		* send a CACHE_READ to local cache
		* expect a CACHE_READ_REPLY
		*/
		else {
			myContext->incCacheHit();
			Message* outMsg = new Message(
				myId, addr, MessageType::CACHE_READ, cacheLatency);
			myContext -> addToCacheMsgQueue(outMsg);
			//cout << "messeging cache for line " << addr << "with type cache read" << endl;

		}

		return;	
	}

}



bool MESIHandler::handleMessage(Message* msg, bool blocked) {
	 	assert(msg->serviced == false);

	 	MessageType type = msg -> msgType;
	 	uint64_t addr = msg -> addr;
	 	int srcId = msg -> sourceID;
		int homeId = myContext -> getHomeNodeIdByAddr(addr);
	 	int myId = myContext -> getContextId();
	 	int numContexts = myContext -> getNumContexts();
	 	// cout << "Context "<<myId <<" recvd a msg " << mString[type] << " from node " << srcId << " for address " << addr << "\n";

	 	MemOp currOp = myContext -> getMemOp();
	 	// printf("currOp action is %d  and addr is %llx \n", currOp.actionType, currOp.addr);
	 	uint64_t opAddr = currOp.addr;

	 	Message* m;
	 	
	 	
	 	switch (type) {
	 		//=============================== READ_MISS ===============================
	 		case READ_MISS:
	 			assert(myId == homeId); // I must be the home node
	 			if (blockedMsgMap.find(addr) == blockedMsgMap.end() || blocked) { // not blocked
	 				msg->serviced = true;
	 				DirectoryEntry entry = myContext -> lookupDirectoryEntry(addr);
	 				if (entry.status == DirectoryEntryStatus::SHARED ) {
	 					sendMsgToNode(srcId, addr, MessageType::DATA_VALUE_REPLY);	 					
	 					myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus::SHARED, srcId);
	 				}
	 				else if (entry.status == DirectoryEntryStatus::UNCACHED) {
	 					sendMsgToNode(srcId, addr, MessageType::DATA_VALUE_REPLY_E);	 					
	 					myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus::EXCLUSIVE, srcId);
	 				}
	 				else if (entry.status == DirectoryEntryStatus::EXCLUSIVE ||
	 					entry.status == DirectoryEntryStatus::MODIFIED) {
	 					/* Even if the line is in EXCLUSIVE state, it's still possible 
	 					*  that the current owner has already modified it. 
	 					*  Hence a FETCH is necessary
	 					*/
	 					int ownerId = 0;
	 					for (bool isOwner : entry.processorMask) {
	 						if (isOwner)	break;
	 						ownerId++;
	 					}
	 					assert(ownerId < numContexts);
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
	 				// cout << "there is no entry for this address in blockedMsgMap \n" ;
	 				DirectoryEntry& entry = myContext -> lookupDirectoryEntry(addr);
	 				if (entry.status == DirectoryEntryStatus::UNCACHED) {
	 					// cout << "sending a DATA_VALUE_REPLY to node" << srcId << "\n";
	 					sendMsgToNode(srcId, addr, MessageType::DATA_VALUE_REPLY);	 					
	 					myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus::MODIFIED, srcId);
	 				} 
	 				else if (entry.status == DirectoryEntryStatus::SHARED) {
	 					int sharerId = 0;
	 					int sharerCount = 0;
	 					for (bool isSharer : entry.processorMask) {
	 						if (isSharer) {
	 							sendMsgToNode(sharerId, addr, MessageType::INVALIDATE);
	 							sharerCount ++;
	 						}
	 						sharerId++;
	 					}
	 					pendingInvAckCount.insert(std::pair<uint64_t, int> (addr, sharerCount));
	 					return false;
	 				}
	 				else {	// MODIFIED OR EXCLUSIVE
	 					// printf("directory entry is already MODIFIED or EXCLUSIVE\n");
	 					int ownerId = 0;
	 					for (int isOwner : entry.processorMask) {
	 						if (isOwner)	break;
	 						ownerId++;
	 					}
	 					assert(ownerId < numContexts);
	 					// cout << "sending a FETCH_INVALIDATE to ownerId " << ownerId << "\n";
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
		 		assert(srcId == homeId);
		 		if (cacheLineStatus.find(addr) == cacheLineStatus.end()) { // invalid
		 			//cout << " The line is not in my cache due to eviction, sending an INVALIDATE_ACK right away \n";
		 			sendMsgToNode(srcId, addr, MessageType::INVALIDATE_ACK);
		 		}
		 		else { 
		 			/* needs to ask cache to invalidate the line and 
		 			 * remove the entry from cacheLineStatus to indicate an INVALID status
		 			 */
		 			// cout << "sending a CACHE_INVALIDATE to cache and removing the entry from cacheLineStatus\n";
		 			cacheLineStatus.erase(addr);
		 			sendMsgToCache(addr, MessageType::CACHE_INVALIDATE);
		 			msg->serviced = true;
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
	 				
					// cout << "Home node has recvd an INVALIDATE_OTHER\n";
	 				msg->serviced = true;
	 				DirectoryEntry entry = myContext -> lookupDirectoryEntry(addr);
	 				if(entry.status != DirectoryEntryStatus::SHARED){
	 					
	 					 /* cannot service this INVALIDATE_OTHER as the directory entry is already
	 					    in modified state and hence somebody else is the owner of this
	 					    line
	 					 */
	 					// cout << "context" << myContext->getContextId() << "sending a ROLLBACK to node " << srcId << "\n";

	 					sendMsgToNode(srcId, addr, MessageType::ROLLBACK);
	 					return true;
	 				}
	 				// cout << "in INVALIDATE, directory status is " << entry.status << endl;
	 				assert(entry.status == DirectoryEntryStatus::SHARED);
	 				int sharerId = 0;
	 				int sharerCount = 0;
	 				for (bool isSharer : entry.processorMask) {
	 					if (isSharer && srcId != sharerId) {
	 						// cout << "sending INVALIDATE to sharer " << sharerId << "\n";
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
	 			 * modify directory status accordingly (by making the original requester exclusive owner)
	 			 * service the first message from the blocked queue
	 			 * try to go down the queue until blocked again
				*/
	 			assert(homeId == myId);
	 			pendingInvAckCount[addr]--;
	 			if (pendingInvAckCount[addr] == 0) {
	 				pendingInvAckCount.erase(addr);
	 				m = blockedMsgMap[addr].front();
	 				blockedMsgMap[addr].erase(blockedMsgMap[addr].begin());
	 				int src = m -> sourceID;
	 				myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus::MODIFIED, src);

	 				// assert - must be either WRITE_MISS or INVALIDATE_OTHER
	 				assert(m->msgType == MessageType::WRITE_MISS || m->msgType == MessageType::INVALIDATE_OTHER);
	 				if (m -> msgType == MessageType::WRITE_MISS) {
	 					// cout << "sending out a DATA_VALUE_REPLY to node " << src << "upon receiving all INVALIDATE_ACKS \n";
	 					sendMsgToNode(src, addr, MessageType::DATA_VALUE_REPLY);	 					
	 				} 
	 				else if (m -> msgType == MessageType::INVALIDATE_OTHER) {
	 					// cout << "sending out an INVALIDATE to node " << srcId << "upon receiving all INVALIDATE_ACKS \n";
	 					sendMsgToNode(src, addr, MessageType::INVALIDATE_OTHER_ACK);	 					
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
	 			assert(srcId == homeId);
	 			assert(opAddr == addr);
	 			cacheLineStatus[addr] = protocolStatus::M;
	 			sendMsgToCache(addr, MessageType::CACHE_UPDATE);
	 			// cout << "sending cache_update for address " << addr << endl;

	 			break;

	 		//=============================== FETCH ===============================
	 		case FETCH:
	 			/* It is possible that cache doesn't have the line anymore due to 
	 			 * previous eviction. We do notify home node about the eviction with
	 			 * a DATA_WRITE_BACK, but home node must have seen a READ request before
	 			 * that and therefore sent us a FETCH. In that case, we ignore this 
	 			 * message because our DATA_WRITE_BACK during eviction will serve as a 
	 			 * response to this FETCH message. */
				if (cacheLineStatus.find(addr) == cacheLineStatus.end()){
					// cout << "returning imm. when we get a FETCH\n" ;
					return true;
				}
				assert(cacheLineStatus[addr] == protocolStatus::M || 
					cacheLineStatus[addr] == protocolStatus::E);
				cacheLineStatus[addr] = protocolStatus::S;
	 			sendMsgToCache(addr, MessageType::CACHE_FETCH);
	 			msg -> serviced = true;
	 			return false;
	 			break;

	 		//=============================== FETCH_INV ===============================
	 		case FETCH_INVALIDATE:
	 			/* Above scenario could also happen here, and we again choose to ignore 
	 			 * this message in that case */
	 			if (cacheLineStatus.find(addr) == cacheLineStatus.end()){
					return true;
				}
				assert(cacheLineStatus[addr] == protocolStatus::M || 
					cacheLineStatus[addr] == protocolStatus::E);
	 			cacheLineStatus.erase(addr);
	 			sendMsgToCache(addr, MessageType::CACHE_INVALIDATE);
	 			msg -> serviced = true;
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
	 			assert(cacheLineStatus.find(addr) == cacheLineStatus.end());
	 			// if a WRITE_MISS
				if (currOp.actionType == contech::action_type::action_type_mem_write) {
					cacheLineStatus.insert(std::pair<uint64_t, protocolStatus> (addr,protocolStatus::M));
				}
				else { // if a READ_MISS
					cacheLineStatus.insert(std::pair<uint64_t, protocolStatus> (addr,protocolStatus::S));
				}
				// cout << "recvd a DATA_VALUE_REPLY, update the cache \n";
				sendMsgToCache(addr, MessageType::CACHE_UPDATE);
	 			break;


	 		//=============================== DATA_VALUE_REPLY_E ===============================
	 		case DATA_VALUE_REPLY_E:
	 			/*
	 			 * This must be a response for a previous READ_MISS we send out to home node
	 			 * change cacheLineStatus[addr] to EXCLUSIVE
	 			 * tell local cache to update the line
				*/
	 			assert(myContext->getSuccessful() == false);
	 			assert(currOp.actionType == contech::action_type::action_type_mem_read);
	 			cacheLineStatus.insert(std::pair<uint64_t, protocolStatus> (addr,protocolStatus::E));
	 			// cout << "recvd a DATA_VALUE_REPLY_E, update the cache \n";
	 			sendMsgToCache(addr, MessageType::CACHE_UPDATE);

	 			break;


	 		//=============================== DATA_WRITE_BACK ===============================
	 		case DATA_WRITE_BACK:
	 			/*
				* We are the home node of the line
				* This must be from exclusive owner (in MODIFIED or EXCLUSIVE), 
				* either replying to a FETCH/FETCH_INVALIDATE we sent earlier
				* or notifying me about eviction
				*
				* Either way, we update directory entry and check the blocked queue for the line
	 			*/
	 			if (blockedMsgMap.find(addr) != blockedMsgMap.end()) {
	 				m = blockedMsgMap[addr].front();
	 				blockedMsgMap[addr].erase(blockedMsgMap[addr].begin());

	 				// assert - must be either WRITE_MISS or READ_MISS
	 				assert(m->msgType == MessageType::WRITE_MISS || 
	 					m->msgType == MessageType::READ_MISS);
	 				if (m -> msgType == MessageType::WRITE_MISS) {
	 					sendMsgToNode(m -> sourceID, addr, MessageType::DATA_VALUE_REPLY);
	 					myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus::MODIFIED, m -> sourceID);
	 				} 
	 				else if (m -> msgType == MessageType::READ_MISS) {
	 					DirectoryEntry entry = myContext -> lookupDirectoryEntry(addr);
	 					if (entry.status == DirectoryEntryStatus::UNCACHED){
		 					sendMsgToNode(m -> sourceID, addr, MessageType::DATA_VALUE_REPLY_E);
		 					myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus::EXCLUSIVE, m -> sourceID); 					
	
	 		 			}
	 		 			else {
		 					sendMsgToNode(m -> sourceID, addr, MessageType::DATA_VALUE_REPLY);
		 					myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus::SHARED, m -> sourceID); 					
	
	 		 			}
	 				}
	 				
	 				checkBlockedQueueAtAddress(addr);
	 			}
	 			else {
	 				myContext -> updateDirectoryEntry(addr, DirectoryEntryStatus::UNCACHED, srcId);
	 			}

	 			break;


	 		//=============================== CACHE_READ_REPLY ===============================	
	 		case CACHE_READ_REPLY:
	 			/* Must be a reply to my own memOp, which is a action_type_read 
	 			 * set successful to 1 so that processor can proceed to next mem op
	 			 */
	 			assert(opAddr == addr);
	 			assert(myContext -> getSuccessful() == false);
	 			myContext -> setSuccessful(true);
	 			break;

	 		//=============================== CACHE_READ_REPLY ===============================	
	 		case CACHE_UPDATE_ACK:
	 			/*
	 			* Indicates a successful update
	 			* The address must match the addr of current mem op
	 			* we can now set successful to true so that processor can proceed
	 			*
	 			* Note that cacheLineStatus is already updated
	 			*/

	 			assert(opAddr == addr);
	 			assert(myContext -> getSuccessful() == false);
	 			myContext -> setSuccessful(true);
	 			break;

	 		//=============================== CACHE_READ_REPLY ===============================	
	 		case CACHE_INVALIDATE_ACK:
	 			/*  
	 			* The first message of the blocked queue must be either an 
	 			* INVALIDATE or FETCH_INVALIDATE. We can now reply to it.
	 			* 
	 			* Note that cacheLineStatus is already updated
	 			*/

	 			m = blockedMsgMap[addr].front();
	 			blockedMsgMap[addr].erase(blockedMsgMap[addr].begin());

	 			assert(m->msgType == MessageType::INVALIDATE || 
	 				m->msgType == MessageType::FETCH_INVALIDATE);
	 			if (m -> msgType == MessageType::INVALIDATE) {
	 				// cout << "recvd a CACHE_INVALIDATE_ACK, sending INVALIDATE_ACK to node " << m->sourceID << "\n";
	 				sendMsgToNode(m -> sourceID, addr, MessageType::INVALIDATE_ACK);	 					
	 			} 
	 			else if (m -> msgType == MessageType::FETCH_INVALIDATE) {
	 			    // cout << "recvd a CACHE_INVALIDATE_ACK, sending DATA_WRITE_BACK to node " << m->sourceID << "\n";
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
	 			assert(m->msgType == MessageType::FETCH);
	 			sendMsgToNode(m -> sourceID, addr, MessageType::DATA_WRITE_BACK);	 
	 			checkBlockedQueueAtAddress(addr);					
	 			break;

	 		//=============================== CACHE_EVICTION_ALERT ===============================	
	 		case CACHE_EVICTION_ALERT:
	 			/*
	 			* if cacheLineStatus was MODIFIED or EXCLUSIVE, need to notify the home node
	 			* change cacheLineStatus to be INVALID by removing the entry
	 			*/
	 			if (cacheLineStatus[addr] == protocolStatus::M || 
	 				cacheLineStatus[addr] == protocolStatus::E) {
					int homeNodeId = myContext -> getHomeNodeIdByAddr(addr);
					sendMsgToNode(homeNodeId, addr, MessageType::DATA_WRITE_BACK);
	 			}
	 			cacheLineStatus.erase(addr);

	 			break;


	 		//=============================== ROLLBACK ==============================================
	 		case ROLLBACK:
 				// cout << "contextId " << myContext->getContextId() << " received a ROLLBACK from node " << srcId << "\n";
 				myContext->reAddCurrMemOp();
 				myContext->setSuccessful(true);
 				break;


	 		default: // UNRECOGNIZED MESSAGE TYPE
	 			cerr << "UNRECOGNIZED MESSAGE TYPE" << endl;
	 			exit(-1);
	 			break;

	 	}
	 	return true;
}




void MESIHandler::checkIncomingMsgQueue() {
	std::vector<Message*>& messages = myContext -> getIncomingMsgQueue();

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
	    	if (!handleMessage(msg, false)) {
	    		// printf("adding to blockedMsgMap\n");
				addToBlockedMsgMap(msg);
			}
		}
		else{
			break;
		}
	}
	messages.erase(messages.begin(), messages.begin() + i);
}
