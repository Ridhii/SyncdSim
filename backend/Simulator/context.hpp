#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "common.hpp"
#include "processor.hpp"
#include "ProtocolHandler.hpp"
#include "directory.hpp"
#include "cache.hpp"


Class Simulator;

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

	std::queue<Message*> cacheMsgQueue;
	std::queue<Message*> incomingMsgQueue;
	std::map<uint64_t, std::queue<Message*> > blockedMsgMap;

	bool successfulMemOp;

   	MemOp currentMemOp;
   	bool currentMemOpSuccessful;

public:
	Context(int contextId, protocolType protocol, Simulator* simulator);
	void run();

	void addToTaskQueue(contech::Task* Task);
	contech::Task* getNextTask(); // return NULL if empty

	void setMemOp(uint64_t addr, contech::action_type type);
	MemOp getMemOp();

	void addCompletedTask(contech::Task* Task);
	vector<contech::Task*>& getCompletedTasks();
	void clearCompletedTasks();

	bool getSuccessful();
	void setSuccessful(bool isSuccessful);

	void addCacheMsg(Message* msg);
	void addToCacheMsgQueue(Message* msg);
	std::queue<Message*>& getCacheMsgQueue();

	void addToIncomingMsgQueue(Message* msg);
	std::queue<Message*>& getIncomingMsgQueue();

	std::map<uint64_t, std::queue<Message*> >& getBlockedMsgMap();

	DirectoryEntry lookupDirectoryEntry(uint64_t addr);
	void updateDirectoryEntry(uint64_t addr, DirectoryEntryStatus status, int pid);

	int getContextId();
	int getHomeNodeIdByAddr(uint64_t addr);
	Context* getContextById(int id);
	int getNumContexts();
};



#endif












