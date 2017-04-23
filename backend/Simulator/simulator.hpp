#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <set>

#include "Task.hpp"
#include "TaskGraph.hpp"

#include "common.hpp"
#include "Processor.hpp"
#include "Directory.hpp"
#include "Cache.hpp"
#include "ProtocolHandler.hpp"
#include "Action.hpp"


class Simulator {
private:
	struct PendingTaskStatus
	{
		unsigned count;
		std::set<TaskId> predecessors;
	};

	TaskGraph* tg;
	int numContexts;
	int numUnfinishedTasks;
	protocolType protocol;
	std::vector<Context> contexts; 
	std::map <TaskId, unsigned int> completedPredecessors;
	std::map <TaskId, PendingTaskStatus> pendingSuccessors;

	int cycleCount;


public:
	Simulator(char* f, protocolType protocol);
	void run();
	void printResult();

};



#endif