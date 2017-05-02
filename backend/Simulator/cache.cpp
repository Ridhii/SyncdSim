#ifndef CACHE_HPP
#define CACHE_HPP



Cache::Cache(int _s, int _e, int _b, Context* _context) {
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

    myContext = _context;

}


/* TODO - Destructor */
Cache::~Cache(){

}


/* Return cache line for the address */
cache_line& Cache::getLine(uint64_t addr) {
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
    int set_idx = strtol (set_bin, &pEnd, 2);

    // search all lines int the set with specified index
    for (int i=0; i<E; ++i){
        if(strncmp(cache[set_idx][i].tag, tag_bin, t) == 0 && cache[set_idx][i].valid == true)// cache hit
        { 
            return cache[set_idx][i];
        }
    }

    return NULL;
}



/* 
 * Cache only gets this request when the line exists 
 * This method is for updating the LRU count for that line
 */
void Cache::readLine(uint64_t addr) {
    cache_line line = getLine(addr);
    line.lru_counter = global_counter++;
}


/* 
 * Set the valid bit of a line to 0
 */
void Cache::invalidateLine(uint64_t addr) {
    cache_line line = getLine(addr);
    line.valid = false;
}


/* ?? */
void Cache::fetchLine(uint64_t addr) {
    cache_line line = getLine(addr);
    line.dirty = false;
    line.lru_counter = global_counter++;
}


/* 
 * If line exists, update its data
 * If not, add the line to cache (might cause eviction)
 * return value indicates whether eviction occurs.
 */
bool Cache::updateLine(uint64_t addr, uint64_t* evictionAddr) {
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
    int set_idx = strtol (set_bin, &pEnd, 2);

    // search all lines int the set with specified index
    int write_idx = -1;
    int min_lru_count = INT_MAX;
    int eviction_idx = -1;
    for (int i=0; i<E; ++i){
        if(strncmp(cache[set_idx][i].tag, tag_bin, t) == 0 && cache[set_idx][i].valid == true) {
            cache[set_idx][i].dirty = true;
            cache[set_idx][i].lru_counter = global_counter ++;
            return false;
        }
        else if (cache[set_idx][i].valid == false) {
            if (write_idx == -1) {
                write_idx = i;
            }
        }
        else {
            if (cache[set_idx][i].lru_counter < min_lru_count) {
                eviction_idx = i;
            }
        }
    }

    if (write_idx != -1) {  // found an empty line to write to
        cache[set_idx][i].valid = true;
        cache[set_idx][i].dirty = true;
        cache[set_idx][i].lru_counter = global_counter ++;
        return false;
    }

    // eviction
    char *tag = cache[set_idx][i].tag;
    addr_bin.replace(addr_bin.begin(), addr_bin.begin() + t, tag);
    char* someChar;  // not sure
    *evictionAddr = strtol (addr_bin, &someChar, 2);
    cache[set_idx][i].lru_counter = global_counter ++;
    return true;

}



/* 
 * Subtract 1 from all cache messages 
 * take care of messages that have reached 0
 */
void Cache::run() {
    queue<Message*> messages = myContext -> getCacheMsgQueue();
    for (Message* msg : messages) {
        msg -> latency --;
        if (msg -> latency == 0) {
            MsgType outMsgType;
            uint64_t addr = msg -> addr;
            switch (msg -> msgType) {
            case CACHE_READ: // request to read a line
                readLine(addr);
                outMsgType = MsgType.CACHE_READ_REPLY;
                break;
            case CACHE_INVALIDATE: // request to invalidate a line
                invalidateLine(addr);
                outMsgType = MsgType.CACHE_INVALIDATE_ACK;
                break;
            case CACHE_FETCH:   // request to read and demote a line to shared (reset dirty)
                fetchLine(addr);
                outMsgType = MsgType.CACHE_FETCH_ACK;
                break;
            case CACHE_UPDATE:  // request to write to a line
                uint64_t evictionAddr;
                bool eviction = updateLine(addr, &evictionAddr);
                outMsgType = MsgType.CACHE_UPDATE_ACK;
                if (eviction) {
                    Message* evictionAlert = new Mesasge(
                        NULL, evictionAddr, CACHE_EVICTION_ALERT, cacheLatency);
                    myContext -> addToIncomingMsgQueue(evictionAlert);
                }
                break;
            default:
                break;

            }

            Message* outMsg = new Message(msg->sourceID, addr, outMsgType, cacheLatency);
            myContext -> addToIncomingMsgQueue(outMsg);
        }
    }
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


