#ifndef CACHE_HPP
#define CACHE_HPP

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <limits.h>
#include <Action.hpp>


typedef struct
{
	int valid;
	char *tag;
	int lru_counter;
} cache_line;


class Cache{
private:
	cache_line** cache;

	long write_hit_count = 0;
	long write_miss_count = 0;
	long read_hit_count = 0;
	long read_miss_count = 0;
	long eviction_count = 0;

	int s, S, E, b, t;

	int global_counter = 0;


public:
	Cache(int _s, int _E, int _b);
	// put msg on incomingMsgQueue with type 
	// CACHE_READ_HIT, CACHE_READ_MISS, CACHE_WRITE_HIT, CACHE_WRITE_MISS
	void checkLine(unint64 addr, action_type type); 

	//	if read:
	//		if no eviction -- reply with updateLineAck
	//		if eviction -- reply with two messages (eviction and Ack)
	// if write:
	// 
	void updateLine(unint64 addr, action_type type);

	// subtract 1 from all cache messages 
	// check if any has reached 0
	// call checkLine or updateLine and compose responses
	void run();
	void printSummary();


};

#endif