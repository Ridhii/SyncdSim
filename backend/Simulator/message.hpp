#ifndef MESSAGE_HPP
#define MESSAGE_HPP


enum MessageType {CACHE_READ, CACHE_READ_REPLY, 
					CACHE_UPDATE, CACHE_UPDATE_ACK,
	              	CACHE_INVALIDATE, CACHE_INVALIDATE_ACK,
	              	CACHE_FETCH, CACHE_FETCH_ACK,
	              	CACHE_EVICTION_ALERT};
struct Message{
    
    /* the ID of the sender */
	int sourceID;
	uint64_t addr;
	MessageType msgType;
	/* latency of communication */
	uint latency;

};




















#endif