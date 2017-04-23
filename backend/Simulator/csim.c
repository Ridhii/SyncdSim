/*
* 15513 Cache Lab Part 1 - Cache Simulator
* Spring 2017
* 
* Name: Yinyi Chen
* Andrew ID: yinyic
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <limits.h>
#include "cachelab.h"

typedef struct
{
	int valid;
	char *tag;
	int lru_counter;
} cache_line;
/*
Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>

-h: Optional help flag that prints usage info
-v: Optional verbose flag that displays trace info
-s <s>: Number of set index bits (S = 2^s is the number of sets)
-E <E>: Associativity (number of lines per set)
-b <b>: Number of block bits (B = 2^b is the block size)
-t <tracefile>: Name of the memory trace to replay
*/

int main (int argc, char *argv[]){
	long hit_count = 0, miss_count = 0, eviction_count = 0;
	int s = 0, E = 0, b = 0, t = 0, debug = 0;
	int global_counter = 0;
	char* tracefile;
    FILE *fp;
    char op;
    long addr;
    int size;

    // use getopt to parse command line options
	int opt;
	while ((opt = getopt(argc, argv, "v::s:E:b:t:")) != -1) {
        switch (opt) {
        case 'v':
        	debug = 1;
        	break;
        case 's':
        	s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            tracefile = malloc(sizeof(char) * strlen(optarg));
            strcpy(tracefile, optarg);
            break;
        default: 
            printf("Invalid arguments!\n");
            return -1;
        }
    }

    t = 64 - s - b;
	// number of sets
    int S = (int)pow(2,s); 

    // cache is a 2-d array (S by E) of cache_line
    cache_line** cache = (cache_line**)malloc(S * sizeof(cache_line*));

    // allocate memory for each set
    for (int i=0; i<S; ++i)
    {
    	cache[i] = (cache_line*)malloc(E * sizeof(cache_line));
    }

	// read tracefile line by line and perform simulation
	// an input line reads:  _Op Addr,Size
	fp = fopen(tracefile,"r");

    while(fscanf(fp, " %c %lx,%d", &op, &addr, &size) > 0)
    {
    	if (debug == 1)
    	{
    		printf("%c %lx,%d\n", op, addr, size);
    	}	

    	char* addr_bin = (char*)malloc(64 * sizeof(char));
    	char* tag_bin = (char*)malloc(t * sizeof(char));
    	char* set_bin = (char*)malloc(s * sizeof(char));
    	char* block_bin = (char*)malloc(b * sizeof(char));

    	// convert long hex to binary
    	for (int i=0; i<64; ++i) 
    	{
  			addr_bin[63-i] = (addr&1)+'0';
  			addr >>= 1;
		}

        // get binary representation of tag, set, and block
		strncpy(tag_bin, addr_bin, t);
		strncpy(set_bin, addr_bin+t, s);
		strncpy(block_bin, addr_bin+t+s, b);

  		char * pEnd;
		int set_idx = strtol (set_bin,&pEnd,2);
		int eviction = 1;
		// search all lines int the set with specified index
		for (int i=0; i<E; ++i){
			if (cache[set_idx][i].valid == 0) // cache miss, no eviction
			{	
				if (debug == 1)
				{
					printf("miss\n");
				}
				miss_count ++;
		        cache[set_idx][i].valid = 1;
		        cache[set_idx][i].tag = (char*)malloc(t * sizeof(char));
		        strncpy(cache[set_idx][i].tag, tag_bin, t);
		        cache[set_idx][i].lru_counter = global_counter++;
		        eviction = 0;
		        break;
			}
			else if(strncmp(cache[set_idx][i].tag, tag_bin, t) == 0)// cache hit
			{ 
				if (debug == 1)
    			{
        			printf("hit\n");
        		}
        		hit_count ++;
        		cache[set_idx][i].lru_counter = global_counter++;
        		eviction = 0;
        		break;
			}
		}

		// Need to evict a LSU line in the set
		if (eviction == 1)
		{
			if (debug == 1)
    		{
        		printf("miss eviction\n");
        	}
        	miss_count ++;
        	eviction_count ++;
        	int min = INT_MAX;
        	int victim = 0;
        	for (int i=0; i < E; ++i)
        	{
        		if (cache[set_idx][i].lru_counter < min)
        		{
        			min = cache[set_idx][i].lru_counter;
        			victim = i;
        		}
        	}	
        	strncpy(cache[set_idx][victim].tag, tag_bin, t);
		    cache[set_idx][victim].lru_counter = global_counter++;
        }

        free(addr_bin);
        free(set_bin);
        free(tag_bin);
        free(block_bin);
    }

    fclose(fp);
    
    // deallocation
    free(tracefile);
    for (int i = 0; i < S; ++i)
    {
    	for (int j = 0; j < E; ++j)
    	{
    		free(cache[i][j].tag);
    	}
    	free(cache[i]); 
    }
    free(cache);

	printSummary(hit_count, miss_count, eviction_count);
	return 0;
}