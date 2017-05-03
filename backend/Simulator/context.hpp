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
	Directory* dir;
	Cache* cache;  
	Simulator* simulator;

	std::queue<Task*> taskQueue;
	std::vector<Task*> completedTasks;

	std::queue<Message*> cacheMsgQueue;
	std::queue<Message*> incomingMsgQueue;

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

	void addCacheMsg(Message* msg);
	vector<Message>& getCacheMsgQueue();
	void addToCacheMsgQueue(Message* msg);
	std::queue<Message*>& getCacheMsgQueue();

	void addToIncomingMsgQueue(Message* msg);
	std::queue<Message*>& getIncomingMsgQueue();

	int getContextId();
	int getHomeNodeIdByAddr(uint64_t addr);
	Context* getContextById(int id);
};



#endif












