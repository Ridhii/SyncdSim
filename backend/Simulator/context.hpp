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
	uint64 address;
	action_type actionType;
};


class Context {

private:
	int contextID;
	Processor* processor;
	ProtocolHandler* pH;
	Directory* dir;
	Cache* cache;  

	std::queue<Task*> taskQueue;
	std::vector<Task*> completedTasks;

	std::queue<Message> cacheMsgQueue;

	bool successfulMemOp;

   	MemOp currentMemOp;
   	bool currentMemOpSuccessful;

public:
	Context(int _contextID, protocolType _protocol);
	void run();

	void addToTaskQueue(Task* _task);
	Task* getNextTask(); // return NULL if empty

	void setMemOp(uint64 _addr, action_type _type);
	MemOp getMemOp();

	void addCompletedTask(Task* _task);
	vector<Task*>& getCompletedTasks();
	void clearCompletedTasks();

	bool getSuccessful();
	void setSuccessful(bool isSuccessful);

	void addCacheMsg(Message msg);
	vector<Message>& getCacheMsgQueue();
};



#endif












