#include "MSIHandler.hpp"

MSIHandler::MSIHandler(Context* context) {
	myContext = context;
}



MSIHandler::~MSIHandler() {

}


void MSIHandler::handleMemOpRequest() {
	MemOp currOp = myContext -> getMemOp();
	uint64_t addr = currOp.addr;

	if (currOp.actionType = action_type.action_type_write) {
		/* 
		*  line not found in map -- INVALID 
		*  send a WRITE_MISS message to home node
		*  expect a DATA_VALUE_REPLY from home node
		*/
		if (cacheLineStatus.find(addr) == cacheLineStatus.end()) {
			Message* outMsg = new Message(
				myContext -> getContextId(), addr, MsgType.WRITE_MISS, nodeLatency);
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
				myContext -> getContextId(), addr, MsgType.INVALIDATE, nodeLatency);
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
				myContext -> getContextId(), addr, MsgType.CACHE_UPDATE, cacheLatency);
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
				myContext -> getContextId(), addr, MsgType.READ_MISS, nodeLatency);
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
				myContext -> getContextId(), addr, MsgType.CACHE_READ, cacheLatency);
			myContext -> addCacheMsg(outMsg);
		}

		return;	
	}

}



void MSIHandler::checkIncomingMsgQueue() {
	queue<Message*> messages = myContext -> getIncomingMsgQueue();


//================================ quite messy below, just copy pasted from google doc ===============



	Message* msg = MsgQueue.start();
Message* nextMsg = msg;
while(msg != NULL){
	msg = nextMsg;
	nextMsg = MsgQueue.getNext();
	MessageType = msg.MessageType();
	Addr               = msg.address();

	if (cacheLine for A is blocked) {
		retrive the map entry for A and append msg to A’s list
		remove msg from incoming_queue
		continue
}

	switch(MessageType){
	Case READ_MISS:
		if(cacheLine for A is shared or uncached in the directory){
			Node* dest = getNodeByID(msg.getSourceID());
			// data is a junk field
Message* replyMsg = new Message(data, addr, 
MessageType.DATA_VALUE_REPLY);
			dest.addToMsgQueue(replyMsg);
			// update the state bit of the cache line in directory
			// take the msg out of the incoming msg queue
		}
		if(cache line in a dirty state){
			 // figure out the owner ID of the cache line
                                   node*	owner = getNodeByID(ownerID)
Message* fetchMsg = New Message(my_id, A, MessageType.FETCH);
			owner.addToMsgQueue(fetchMsg);
			
			// And update the state of cache line to be Blocked
			// Get the map entry for A and append the message to the list
			// remove msg from incoming msg queue
		}
                        
		break;

	Case WRITE_MISS:
		If(cache line is in uncached state in the directory){
			Node* dest = getNodeByID(msg.getSourceID());
			// data is a junk field
Message* replyMsg = new Message(data, addr, 
MessageType.DATA_VALUE_REPLY);
			dest.addToMsgQueue(replyMsg);
			//take the msg out of the incoming msg queue

			// update the cache line to be in dirty state in the directory	
		}
		if(cache line is in a shared state){
			For all the sharers of the cache line send out an invalidation msg to each 
			And update the state of cache line to be Blocked
			// Get the map entry for A and append the message to the list
			// remove msg from incoming msg queue
		}
		if(cache line is in a modified state){
                                   // figure out the owner ID of the cache line
                                   node*	owner = getNodeByID(ownerID)
Message* fetchInvMsg = New Message(my_id, A, MessageType.FETCH_INVALIDATE);
			owner.addToMsgQueue(fetchInvMsg);
			
			// And update the state of cache line to be Blocked
			// Get the map entry for A and append the message to the list
			// remove msg from incoming msg queue
		}
		
	Case INVALIDATE:
// if I am the home node, this request is for me to send out further INVALIDATE requests to the sharers except for the sender. 
// remove the msg from the incoming queue and add it to the blocked list queue for the map entry of A.
Also, block the line

		// if I am not the home node, the request is for me to invalidate my cache line and 		ACK	
// remove the msg from incoming_queue
// check the status of the line: if SHARED -- send message to cache to invalidate, and upon receiving an ACK from CACHE,send INVALID_ACK to the home node. else if the line is INVALID -- just send INVALID_ACK to home node immediately (cause we don’t have the line in the cache)


Case FETCH:
// change the state of the cache line in pH from modified to shared.
// tell cache to unset the dirty bit and send a CACHE_FETCH
// upon receiving the CACHE_FETCH_ACK, send a Data Write Back Fetch msg to the home node
(what if my processor wants to write the line in the meantime?)

	Case FETCH_INVALIDATE
// change the state of the cache line in pH from modified to invalid.
// tell cache to unset the dirty bit and send a CACHE_FETCH_INV
// upon receiving the CACHE_FETCH_INV_ACK, send a Data Write Back Fetch Invalidate msg to the home node

	Case DATA_VAL_REPLY
		// you must be the requesting node and successful must be 0 right now
		// send an UPDATE_LINE to cache and wait for UPDATE_LINE_ACK
		// satisfy the curr request and set successful = 1
		

	Case DATA_WRITE_BACK_FETCH:
// you must be the home node, this cache line must have been in a dirty state when the FETCH msg was sent and still is
// update the state of the cache line to be shared
// unblock the line
// retrieve the list in the map for A
while (not blocked again) {
	take the next msg from the list
	
}

// and check the blocked_msg map for the associated msg that causes the block in the first place
// send a DATA_VAL_REPLY
// remove 
//unblock the cache line

Case DATA_WRITE_BACK_FETCH_INV:
// you must be the home node, this cache line must have been in a dirty state when the FETCH_INV msg was sent and still is
// remove the msg at the head ( the msg that caused the blocked), iterate tho



	}
}

}
}