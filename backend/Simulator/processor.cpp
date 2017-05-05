#include "processor.hpp"

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
	while (iter != memOps.end()){
        contech::MemoryAction ma = *(iter);
        uint64_t addr = ma.addr;
        uint64_t alignedAddr = addr & ALIGNER;
        memActionQueue.push(ma);
        
        /* if the addr spans two cache lines, separate the memory action into 
           two acif(tions
        */
        if(addr + ACTION_SIZE > alignedAddr + LINE_SIZE){
        	contech::MemoryAction newMemAction{(uint64_t)(alignedAddr + LINE_SIZE),
        		(uint64_t)POW_SIZE, (contech::action_type)ma.type};
        	memActionQueue.push(newMemAction);
        }
        iter++;
    }
}

void Processor::run(){

	while(myContext->getSuccessful()){
		myContext->setSuccessful(false);
		if (!memActionQueue.empty()) {
			contech::MemoryAction ma = memActionQueue.front();
			memActionQueue.pop();
			/* align the ma.addr to cache line size */
			ma.addr &= ALIGNER;
			myContext->setMemOp(ma.addr, (contech::action_type)ma.type);
		}
		else{
			if(currTask != NULL){
				myContext->addCompletedTask(currTask);

			}
			currTask = myContext->getNextTask();
			myContext->setSuccessful(true);
			if(currTask != NULL){
				populateMemActionQueue();
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



