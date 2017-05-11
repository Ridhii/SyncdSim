#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "common.hpp"
#include "processor.hpp"
#include "protocolHandler.hpp"
#include "MSIHandler.hpp"
#include "MESIHandler.hpp"
#include "directory.hpp"
#include "cache.hpp"
#include "simulator.hpp"

class Simulator;
class Processor;
class ProtocolHandler;
class MSIHandler;
class Directory;
class Cache;
class Directory;


struct MemOp
{
	uint64_t addr;
	contech::action_type actionType;
};


class Context {

private:
	int contextId;
	Processor* processor;
	ProtocolHandler* pH;
	Directory* directory;
	Cache* cache;  
	Simulator* simulator;

	std::queue<contech::Task*> taskQueue;
	std::vector<contech::Task*> completedTasks;

	std::vector<Message*> cacheMsgQueue;
	std::vector<Message*> incomingMsgQueue;
	std::map<uint64_t, std::vector<Message*> > blockedMsgMap;

   	MemOp currentMemOp;
   	bool currentMemOpSuccessful;

   	/* Analysis */
   	int cacheHit;
   	int cacheMiss;
   	/* count of numMsgs sent to other nodes */
   	int numSentMsgs;
   	int numInvalidationsSent;

public:
	Context(int contextId, protocolType protocol, Simulator* simulator);
	void run();

	void addToTaskQueue(contech::Task* Task);
	contech::Task* getNextTask(); // return NULL if empty

	void setMemOp(uint64_t addr, contech::action_type type);
	MemOp& getMemOp();

	void addCompletedTask(contech::Task* Task);
	vector<contech::Task*>& getCompletedTasks();
	void clearCompletedTasks();

	bool getSuccessful();
	void setSuccessful(bool isSuccessful);

	void addToCacheMsgQueue(Message* msg);
	std::vector<Message*>& getCacheMsgQueue();

	void addToIncomingMsgQueue(Message* msg);
	std::vector<Message*>& getIncomingMsgQueue();

	std::map<uint64_t, std::vector<Message*> >& getBlockedMsgMap();

	DirectoryEntry& lookupDirectoryEntry(uint64_t addr);
	void updateDirectoryEntry(uint64_t addr, DirectoryEntryStatus status, int pid);

	int getContextId();
	int getHomeNodeIdByAddr(uint64_t addr);
	Context* getContextById(int id);
	int getNumContexts();

	void handleMemOpRequest();
	/* Analysis */
	void incCacheHit();
	void incCacheMiss();
	void incNumSentMsgs();
	void incNumInvalidations();
	void printContextStats();
};



#endif












