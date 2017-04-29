#ifndef COMMON_HPP
#define COMMON_HPP

enum protocolType { MSI, MESI };
extern protocolType protocol;
/* these must be defined in simulator via the arguments from the 
   command line
*/

/* taskgraph file from the user */
extern char* f;
extern int nodeLatency;
extern int cacheLatency;


#endif