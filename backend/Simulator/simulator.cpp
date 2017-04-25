#include "Simulator.hpp"
#include <cmath>

Simulator::Simulator(char* f, protocolType protocol){
	cycleCount = 0;
	tg = TaskGraph::initFromFile(f);
	numContexts = tg -> getNumberOfContexts();
	numContexts = pow(2, floor(log(numContexts)/log(2))); // round down to nearest power of 2
	numUnfinishedTasks = tg -> getNumberOfTasks();

	// initialize contexts
	for (int i = 0; i < numContexts; ++i){
		contexts.emplace_back(Context(i, protocol));
	}

	// assign the very first task to processor
	Task* firstTask = tg -> getNextTask();
	unsigned cid = (uint32_t)(firstTask -> getContextId());
	/* TODO: cid = cid % numContexts */
	contexts[cid].addToTaskQueue(firstTask);

}

void Simulator::run(){
	while (numUnfinishedTasks != 0) {
		cycleCount ++;
		// run all processors and protocolHandlers
		for (Context c : contexts){
			c.run();
			// check if a context completed any tasks in the tick
			// NOTE: PROCESSOR WILL HAVE TO CLEAR THIS LIST AT THE BEGINNING OF EACH TICK
			for (Task* t : c.getCompletedTasks()){
				numUnfinishedTasks --;
				std::vector<TaskId> successors = t -> getSuccessorTasks();
				if (successors.size() != 0) {
					TaskId predecessorTid = t -> getTaskId();
					// add the completed task and number of its successors to map
					completedPredecessors.insert( std::pair<TaskId, unsigned int>(predecessorTid, successors.size()));
					
					for (TaskId successorTid : successors) {
						// this successor has been added by its another predecessor
						if (pendingSuccessors.find(successorTid) != pendingSuccessors.end()) {
							// remove this completed task from its predecessor set
							pendingSuccessors[successorTid].count--;
						}
						// the successor hasn't been added
						else {	
							// create map entry for this successor
							Task* newSuccessor = TaskGraph::getTaskById(successorTid);
							std::vector<TaskId> preds = newSuccessor -> getPredecessorTasks();
							PendingTaskStatus s;
							std::set<TaskId> s(preds.begin(), preds.end());
							PendingTaskStatus status{preds.size() - 1, s}; // subtract 1 due to the completion of this predecessor
							pendingSuccessors.insert(std::pair<TaskId, PendingTaskStatus> (successorTid, status));
						}

						// the successor is ready to run since all its predesssors have completed
						if (pendingSuccessors[successorTid].count == 0){
							// assign it to the corresponding context
							Task* newTask = TaskGraph::getTaskById(successorTid);
							unsigned cid = (uint32_t)(newTask -> getContextId());
							/* TODO: cid = cid % numContexts */
							contexts[cid].addToTaskQueue(newTask);
							// iterate through predecessor set and
							// decrement child count for all its predecessors
							for(auto pred : pendingSuccessors[successorTid].predecessors) {
	  							completedPredecessors[pred] --;
							}    
							// remove map entry
							pendingSuccessors.erase(successorTid);
						}
					} 
				}
			} // end for completed tasks
		} // end for contexts
	} // end while
}

void Simulator::printResult(){
	printf("Simulation is completed\n");
}









