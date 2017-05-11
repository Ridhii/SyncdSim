#include "cache.hpp"

Cache::Cache(Context* context) {
    myContext = context;

    s = 2; // 2 set bits, i.e. 4 sets in total
    E = 4; // 4 lines every set
    b = 9; // block bits, i.e. each line has 64 byte

    t = 64 - s - b;
    S = (int)pow(2,s); 

    cache = (cache_line**)malloc(S * sizeof(cache_line*));

    for (int i=0; i<S; ++i)
    {
        cache[i] = (cache_line*)malloc(E * sizeof(cache_line));
    }
}

Cache::Cache(int _s, int _e, int _b, Context* _context) {
    s = _s;
    E = _e;
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
Cache::~Cache(){}


/* Return cache line for the address */
cache_line& Cache::getLine(uint64_t addr) {
    // tag | set | block
    unsigned tag = addr >> (s+b);
    unsigned set_idx = (addr << t) >> (t+b);

    // search all lines in the set with specified index
    for (int i=0; i<E; ++i){
        if(cache[set_idx][i].tag == tag && cache[set_idx][i].valid == true)// cache hit
        { 
            return cache[set_idx][i];
        }
    }
    // throw exception

    std::cerr << "cache line not found, quitting" << std::endl;
    exit(-1);
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
        // tag | set | block

    unsigned tag = addr >> (s+b);
    unsigned set_idx = (addr << t) >> (t+b);

    int write_idx = -1;
    int min_lru = INT_MAX;
    int eviction_idx = -1;

    // search all lines int the set with specified index
    for (int i=0; i<E; ++i){
        if(cache[set_idx][i].tag == tag && cache[set_idx][i].valid == true)// cache hit
        { 
            cache[set_idx][i].dirty = true;
            cache[set_idx][i].lru_counter = global_counter ++;
            return false;
        }
        else if (cache[set_idx][i].valid == false) { // an empty line to write to
            if (write_idx == -1) {
                write_idx = i;
            }
        }
        else {  // candidate to evict
            if (cache[set_idx][i].lru_counter < min_lru) {
                eviction_idx = i;
            }
        }
    }

    if (write_idx != -1) {  // found an empty line to write to
        cache[set_idx][write_idx].valid = true;
        cache[set_idx][write_idx].dirty = true;
        cache[set_idx][write_idx].lru_counter = global_counter ++;
        cache[set_idx][write_idx].tag = tag;
        return false;
    }

    // eviction
    unsigned evicted_tag = cache[set_idx][eviction_idx].tag;

    cache[set_idx][eviction_idx].lru_counter = global_counter ++;
    cache[set_idx][eviction_idx].tag = tag;
    cache[set_idx][eviction_idx].dirty = true;
    cache[set_idx][eviction_idx].valid = true;

    *evictionAddr = ((addr << t) >> t) | (evicted_tag << (b + s));
    return true;
}



/* 
 * Subtract 1 from all cache messages 
 * take care of messages that have reached 0
 */
void Cache::run() {
    std::vector<Message*>& messages = myContext -> getCacheMsgQueue();
    //cout << "cache message queue size: " << messages.size() << endl;
    for (Message* msg : messages) {
        if (msg -> latency > 0) {
            msg -> latency--;
        }
    }

    Message* msg = messages.front();
    while (!messages.empty() && msg -> latency == 0) {
            MessageType outMsgType;
            uint64_t addr = msg -> addr;
            bool eviction;
            switch (msg -> msgType) {
            case CACHE_READ: // request to read a line
                readLine(addr);
                outMsgType = MessageType::CACHE_READ_REPLY;
                break;
            case CACHE_INVALIDATE: // request to invalidate a line
                invalidateLine(addr);
                outMsgType = MessageType::CACHE_INVALIDATE_ACK;
                break;
            case CACHE_FETCH:   // request to read and demote a line to shared (reset dirty)
                fetchLine(addr);
                outMsgType = MessageType::CACHE_FETCH_ACK;
                break;
            case CACHE_UPDATE:  // request to write to a line
                uint64_t evictionAddr;
                eviction = updateLine(addr, &evictionAddr);
                outMsgType = MessageType::CACHE_UPDATE_ACK;
                if (eviction) {
                    Message* evictionAlert = new Message(
                        0, evictionAddr, CACHE_EVICTION_ALERT, cacheLatency);
                    myContext -> addToIncomingMsgQueue(evictionAlert);
                }
                break;
            default:
                break;

            }

            Message* outMsg = new Message(msg->sourceID, addr, outMsgType, cacheLatency);
            myContext -> addToIncomingMsgQueue(outMsg);

            messages.erase(messages.begin());
            msg = messages.front();
    }
}



void Cache::printSummary(long hits, long misses, long evictions)
{
    printf("hits:%ld misses:%ld evictions:%ld\n", hits, misses, evictions);
}


