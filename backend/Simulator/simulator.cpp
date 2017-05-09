#include "simulator.hpp"



Simulator::Simulator(char* f, protocolType protocol){
	cycleCount = 0;
	printf("initialize a simulator\n");
	tg = contech::TaskGraph::initFromFile(f);
	numContexts = tg -> getNumberOfContexts();
	numContexts = 4; // pow(2, floor(log(numContexts)/log(2))); // round down to nearest power of 2
    #ifdef DEBUG
	printf("Number of rounded contexts is %d \n", numContexts);
	#endif
	
	numUnfinishedTasks = tg -> getNumberOfTasks();
	// FOR DEBUGGING
	numUnfinishedTasks = 10;

	// initialize contexts
	for (int i = 0; i < numContexts; ++i){
		Context* newContext = new Context(i, protocol, this);
		contexts.push_back(newContext);
	}

	// assign the very first contech::Task to processor
	contech::Task* firstTask = tg -> getNextTask();
	unsigned cid = (uint32_t)(firstTask -> getContextId());
	// mod cid 
	cid = cid % numContexts;
	contexts[cid]->addToTaskQueue(firstTask);
	printf("Simulator created\n");

}

void Simulator::run(){
	/* changed this from != 0 in case we reach negative num of unfinished tasks, although that shouldn't 
	   happen
	*/
	while (numUnfinishedTasks > 0) {
		cycleCount ++;
		printf("--------------- Cycle Count %d -------------------------\n", cycleCount);
		printf("------------------------------------------------------\n");
		// run all processors and protocolHandlers
		for (Context* c : contexts){
			c->run();
			// check if a context completed any Tasks in the tick
			for (contech::Task* t : c->getCompletedTasks()){
				numUnfinishedTasks --;
				printf("******* TASK FINISHED, LEFT %d ********\n", numUnfinishedTasks);
				std::vector<contech::TaskId> successors = t -> getSuccessorTasks();
				printf("successors size is %lu\n", successors.size());
				if (successors.size() != 0) {
					printf("getting successors\n");
					contech::TaskId predecessorTid = t -> getTaskId();
					// add the completed contech::Task and number of its successors to map
					completedPredecessors.insert( std::pair<contech::TaskId, unsigned int>(predecessorTid, successors.size()));
					
					for (contech::TaskId successorTid : successors) {
						// this successor has been added by its another predecessor
						if (pendingSuccessors.find(successorTid) != pendingSuccessors.end()) {
							// remove this completed Task from its predecessor set
							pendingSuccessors[successorTid].count--;
						}
						// the successor hasn't been added
						else {	
							// create map entry for this successor
							printf("creating a map entry for successor\n");
							contech::Task* newSuccessor = tg -> getTaskById(successorTid);
							std::vector<contech::TaskId> preds = newSuccessor -> getPredecessorTasks();
							std::set<contech::TaskId> s(preds.begin(), preds.end());
							PendingTaskStatus status{preds.size() - 1, s}; // subtract 1 due to the completion of this predecessor
							pendingSuccessors.insert(std::pair<contech::TaskId, PendingTaskStatus> (successorTid, status));
						}

						// the successor is ready to run since all its predesssors have completed
						if (pendingSuccessors[successorTid].count == 0){
							// assign it to the corresponding context
							contech::Task* newTask = tg -> getTaskById(successorTid);
							unsigned cid = (uint32_t)(newTask -> getContextId());
							cid = cid % numContexts;
							printf("assigning the successor task to it's context %d\n", cid);
							contexts[cid] -> addToTaskQueue(newTask);
							// iterate through predecessor set and
							// decrement child count for all its predecessors
							for(auto pred : pendingSuccessors[successorTid].predecessors) {
	  							completedPredecessors[pred] --;
	  							if(completedPredecessors[pred] == 0){
	  								completedPredecessors.erase(pred);
	  							}
							}    
							// remove map entry
							pendingSuccessors.erase(successorTid);
						}
					} 
				}
			} // end for completed Tasks
		} // end for contexts
	} // end while
	printResult();

}

void Simulator::printResult(){
	printf("Simulation is completed\n");
}

int Simulator::getNumContexts() {
	return numContexts;
}

Context* Simulator::getContextById(int id) {
	return contexts[id];
}










