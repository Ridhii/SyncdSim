#include "common.hpp"
#include "simulator.hpp"
#include <stdio.h>
#define DEBUG
int nodeLatency;
int cacheLatency;
protocolType protocol;

int main(int argc, char** argv)
{
    if (argc == 3)
    {   
        protocol = MSI;
        cacheLatency = 2;
        nodeLatency  = 1;
        Simulator simulator = Simulator(argv[0], protocol);
        simulator.run();
    }


    else {
        fprintf(stderr, "%s <taskgraph>\n", argv[0]);
        return 1;
    }

    return 0;
}