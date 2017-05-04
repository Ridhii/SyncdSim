#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <stdint.h>


enum MessageType {CACHE_READ, 
					CACHE_READ_REPLY, 
					CACHE_UPDATE, 
					CACHE_UPDATE_ACK,
	              	CACHE_INVALIDATE, 
	              	CACHE_INVALIDATE_ACK,
	              	CACHE_FETCH, 
	              	CACHE_FETCH_ACK,
	              	CACHE_EVICTION_ALERT,
	              	WRITE_MISS, 
	              	READ_MISS, 
	              	INVALIDATE,
	              	INVALIDATE_ACK,
	              	FETCH,
	              	FETCH_INVALIDATE,
	              	DATA_VALUE_REPLY,
	              	DATA_WRITE_BACK
	              	};
struct Message{
    
    /* the ID of the sender */
	int sourceID;
	uint64_t addr;
	MessageType msgType;
	/* latency of communication */
	unsigned latency;

};




















#endif