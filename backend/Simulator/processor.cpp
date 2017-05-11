#include "processor.hpp"
int taskStarted;

Processor::Processor(Context* context){
    
    /* initializing member myContext to point to the context the
       processor belongs to
    */
	myContext = context;
	currTask  = NULL;
}

void Processor::populateMemActionQueue(){
	assert(currTask != NULL);
	/* only called to populate the memActionQueue with the 
	   currTask's memOps
	*/
	contech::Task::memOpCollection memOps = currTask->getMemOps();
	auto iter = memOps.begin();
	int index = 0;
	while (iter != memOps.end()){
        contech::MemoryAction ma = *(iter);
        uint64_t addr = ma.addr;
        uint64_t alignedAddr = addr & ALIGNER;
        memActionQueue.push(ma);
        
        //if the addr spans two cache lines, separate the memory action into 
        //two actions
        if(addr + ACTION_SIZE > alignedAddr + LINE_SIZE){
        	contech::MemoryAction newMemAction{(uint64_t)(alignedAddr + LINE_SIZE),
        		                               (uint64_t)POW_SIZE, (contech::action_type)ma.type};
        	memActionQueue.push(newMemAction);
        }
        iter++;
        index++;
        //FOR DEBUGGING/TESTING
        if(index == ITER_CAP){
        	break;
        }
    }
}

void Processor::run(){

	while(myContext->getSuccessful()){
		if(numTasksLeft == 39) cout << "entering the while loop in processor.run()\n";
		myContext->setSuccessful(false);
		if (!memActionQueue.empty()) {
			contech::MemoryAction ma = memActionQueue.front();
			memActionQueue.pop();
			/* align the ma.addr to cache line size */
			ma.addr = ma.addr & ALIGNER;
			//cout << "memory action is " << ma.type << " and" << std::hex << " addr is " << ma.addr << "\n";
			myContext->setMemOp(ma.addr, (contech::action_type)ma.type);
		}
		else{
			if(currTask != NULL){
				cout << " ******** CONTEXT " << myContext->getContextId() << " FINISHES IT'S TASK ********\n";
				myContext->addCompletedTask(currTask);
				currTask = NULL;

			}
			//printf("getting the next task \n");
			currTask = myContext->getNextTask();
			myContext->setSuccessful(true);
			if(currTask != NULL){
				populateMemActionQueue();
				if(numTasksLeft == 39) cout << "populateMemActionQueue returns \n";
				taskStarted += 1;
				cout << " ******** TASK " << taskStarted << " STARTS IN CONTEXT" << myContext->getContextId() << " ******** \n";
				continue;

			}
			else{
				return;
			}
		}
		myContext -> handleMemOpRequest();
	}
	return;
}



