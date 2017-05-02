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

#include "common.h"
#include "context.hpp"


typedef struct
{
	bool valid;
	bool dirty;
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

	Context* myContext;

	cache_line& getLine(uint64_t addr);
	void readLine(uint64_t addr);
	void invalidateLine(uint64_t addr);
	void fetchLine(uint64_t addr);
	void updateLine(uint64_t addr, uint64_t* evictionAddr);


public:
	Cache(int _s, int _E, int _b, Context* context);
	void run();
	void printSummary();


};

#endif