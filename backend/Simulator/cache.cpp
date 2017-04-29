#ifndef CACHE_HPP
#define CACHE_HPP



Cache::Cache(int _s, int _e, int _b) {
    s = _s;
    e = _e;
    b = _b;
    t = 64 - s - b;
    S = (int)pow(2,s); 

    cache = (cache_line**)malloc(S * sizeof(cache_line*));

    for (int i=0; i<S; ++i)
    {
        cache[i] = (cache_line*)malloc(E * sizeof(cache_line));
    }

}

void Cache::checkLine(unint64 addr, action_type type){
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
            // miss_count ++;
            cache[set_idx][i].valid = 1;
            cache[set_idx][i].tag = (char*)malloc(t * sizeof(char));
            strncpy(cache[set_idx][i].tag, tag_bin, t);
            cache[set_idx][i].lru_counter = global_counter++;
            eviction = 0;
            break;
        }
        else if(strncmp(cache[set_idx][i].tag, tag_bin, t) == 0)// cache hit
        { 
            if (type == action_type::action_type_mem_read)
                // hit_count ++;
            cache[set_idx][i].lru_counter = global_counter++;
            eviction = 0;
            break;
        }
    }



    // TODO: who should keep the data structure of the "protocol state" (i.e. M,S,I for cache lines?) 
}

void Cache::updateLine(unint64 addr, action_type type){

}


void Cache::printSummary(long hits, long misses, long evictions)
{
    printf("hits:%ld misses:%ld evictions:%ld\n", hits, misses, evictions);
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%ld %ld %ld\n", hits, misses, evictions);
    fclose(output_fp);
}


#endif 


