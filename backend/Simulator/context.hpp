#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "processor.hpp"
#include "ProtocolHandler.hpp"
#include "directory.hpp"
#include "cache.hpp"
#include "common.hpp"
#include "Action.hpp"

struct MemOp
{
	uint64_t addr;
	action_type actionType;
};


class Context {

private:
	int contextId;
	Processor* processor;
	ProtocolHandler* pH;
	Directory* directory;
	Cache* cache;  
	Simulator* simulator;

	std::queue<Task*> taskQueue;
	std::vector<Task*> completedTasks;

	std::queue<Message*> cacheMsgQueue;
	std::queue<Message*> incomingMsgQueue;
	std::map<uint64_t, std::queue<Message*> > blockedMsgMap;

	bool successfulMemOp;

   	MemOp currentMemOp;
   	bool currentMemOpSuccessful;

public:
	Context(int contextId, protocolType protocol, Simulator* simulator);
	void run();

	void addToTaskQueue(Task* task);
	Task* getNextTask(); // return NULL if empty

	void setMemOp(uint64 addr, action_type type);
	MemOp getMemOp();

	void addCompletedTask(Task* task);
	vector<Task*>& getCompletedTasks();
	void clearCompletedTasks();

	bool getSuccessful();
	void setSuccessful(bool isSuccessful);

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












