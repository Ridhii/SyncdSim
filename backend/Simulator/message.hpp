#ifndef MESSAGE_HPP
#define MESSAGE_HPP


enum CacheMessage {CACHE_READ,UPDATE_LINE, UPDATE_LINE_ACK,
	              CACHE_READ_REPLY, INVALIDATE_LINE, INVALIDATE_LINE_ACK
	              CACHE_FETCH, CACHE_FETCH_ACK};
struct message{
    
    /* the ID of the sender */
	int sourceID;
	uint64_t addr;
	MessageType msgType;
	/* latency of communication */
	uint latency;


};




















#endif