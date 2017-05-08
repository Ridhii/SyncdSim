#include "common.hpp"
#include "simulator.hpp"
#include <stdio.h>

int nodeLatency;
int cacheLatency;
protocolType protocol;

int main(int argc, char** argv)
{
    /* we expect 4 input args, taskGraph file, protocolType,
       node and cache latency
    */
    printf("argc = %d and argv[1] = %s\n", argc, argv[1]);
    if (argc == 5)
    {   
        protocol = protocolType::MSI; //argv[2s]
        nodeLatency  = std::atoi(argv[3]);
        cacheLatency = std::atoi(argv[4]);
        #ifdef DEBUG
        printf("taskGraph is %s, node latency is %d and cache latency is %d\n", argv[0], nodeLatency, cacheLatency);
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