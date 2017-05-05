#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP
#include "common.hpp"
#include "context.hpp"

class Simulator {
private:
	struct PendingTaskStatus
	{
		unsigned long count;
		std::set<contech::TaskId> predecessors;
	};

	contech::TaskGraph* tg;
	int numContexts;
	int numUnfinishedTasks;
	protocolType protocol;
	std::vector<Context*> contexts; 
	std::map <contech::TaskId, unsigned long> completedPredecessors;
	std::map <contech::TaskId, PendingTaskStatus> pendingSuccessors;

	int cycleCount;


public:
	Simulator(char* f, protocolType protocol);
	void run();
	void printResult();
	Context* getContextById(int id);
	int getNumContexts();

};



#endif