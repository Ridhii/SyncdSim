#include "common.hpp"
#include "simulator.hpp"
#include <stdio.h>

int nodeLatency;
int cacheLatency;
protocolType protocol;

const char* mString[] = {"CACHE_READ", 
                    "CACHE_READ_REPLY", 
                    "CACHE_UPDATE", 
                    "CACHE_UPDATE_ACK",
                    "CACHE_INVALIDATE", 
                    "CACHE_INVALIDATE_ACK",
                    "CACHE_FETCH", 
                    "CACHE_FETCH_ACK",
                    "CACHE_EVICTION_ALERT",
                    "WRITE_MISS", 
                    "READ_MISS", 
                    "INVALIDATE",
                    "INVALIDATE_ACK",
                    "FETCH",
                    "FETCH_INVALIDATE",
                    "DATA_VALUE_REPLY",
                    "DATA_VALUE_REPLY_E",
                    "DATA_WRITE_BACK"

  };
  
int main(int argc, char** argv)
{
    /* we expect 4 input args, taskGraph file, protocolType,
       node and cache latency
    */
    printf("argc = %d and argv[1] = %s\n", argc, argv[1]);
    if (argc == 5)
    {   
        /* max length of string is 5, should change as more protocols are added!!*/
        if(std::strncmp(argv[2],"MSI", 5) == 0){
            cout << "protocol == MSI\n";
            protocol = protocolType::MSI; //argv[2]

        }
        else{
            cout << "protocol == MESI\n";
            protocol = protocolType::MESI;
        }
        nodeLatency  = std::atoi(argv[3]);
        cacheLatency = std::atoi(argv[4]);
        #ifdef DEBUG
        printf("taskGraph is %s, node latency is %d and cache latency is %d and ITER_CAP is %d\n", argv[0], nodeLatency, cacheLatency, ITER_CAP);
        #endif
        Simulator simulator = Simulator(argv[1], protocol);
        simulator.run();
    }


    else {
        fprintf(stderr, "%s <taskgraph>\n", argv[0]);
        return 1;
    }

    return 0;
}