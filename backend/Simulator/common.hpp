#ifndef COMMON_HPP
#define COMMON_HPP

#include <stdint.h>
#include <assert.h>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <limits.h>
#include <cmath>
#include <set>
#include <queue>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "../../common/taskLib/Action.hpp"
#include "../../common/taskLib/Task.hpp"
#include "../../common/taskLib/TaskGraph.hpp"
#include "message.hpp"

#define ALIGNER 0xFFFFFE00
#define LINE_SIZE 64
#define ACTION_SIZE 8
#define POW_SIZE 3

enum protocolType { MSI, MESI };
extern protocolType protocol;
/* these must be defined in simulator via the arguments from the 
   command line
*/

/* contech::Taskgraph file from the user */
extern char* f;
extern int nodeLatency;
extern int cacheLatency;


#endif