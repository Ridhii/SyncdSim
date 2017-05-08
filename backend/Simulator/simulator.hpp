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
	/* A map of completed tasks and the number of their pending successors */
	std::map <contech::TaskId, unsigned long> completedPredecessors;
	/* A map of succesor that maps to a struct. The struct has a parent count.
	   Everytime, a parent finishes, we decrease this count. The struct also
	   has a set which is constant of all the parents of that task
	*/
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