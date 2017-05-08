#include "processor.hpp"
//FOR DEBUGGing
int tempTotalTask = 0;

Processor::Processor(Context* context){
    
    /* initializing member myContext to point to the context the
       processor belongs to
    */
	myContext = context;
	currTask  = NULL;
}

void Processor::populateMemActionQueue(){
	assert(currTask != NULL);
	printf("in populateMemActionQueue \n");
	/* only called to populate the memActionQueue with the 
	   currTask's memOps
	*/
	contech::Task::memOpCollection memOps = currTask->getMemOps();
	auto iter = memOps.begin();
	/* FOR DEBUGGING */
	int myID = myContext->getContextId();
	uint64_t fakeAddr;
	if(myID == 0){
		fakeAddr = (0x8bcdabcdabcd);

	}
	if(myID == 1){
		fakeAddr = 0x0bcdabcdabcd;
		
	}
	if(myID == 2){
		fakeAddr = 0x0bcdabcdabcd;
		
	}
	if(myID == 3){
		fakeAddr = 0xcbcdabcdabcd;
		
	}
	contech::MemoryAction newMemAction{(uint64_t)fakeAddr,
        		                       (uint64_t)POW_SIZE, (contech::action_type)(contech::action_type_mem_write)};
    memActionQueue.push(newMemAction);
	/*while (iter != memOps.end()){
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
    }
    */
}

void Processor::run(){

	while(myContext->getSuccessful() && (tempTotalTask == 0)){
		myContext->setSuccessful(false);
		if (!memActionQueue.empty()) {
			contech::MemoryAction ma = memActionQueue.front();
			memActionQueue.pop();
			/* align the ma.addr to cache line size */
			cout << "memory action is " << ma.type << " and" << std::hex << " addr is " << ma.addr << "\n";
			ma.addr = ma.addr & ALIGNER;
			cout << "memory action is " << ma.type << " and" << std::hex << " addr is " << ma.addr << "\n";
			myContext->setMemOp(ma.addr, (contech::action_type)ma.type);
		}
		else{
			if(currTask != NULL){
				myContext->addCompletedTask(currTask);
				tempTotalTask += 1;
				if(tempTotalTask == 1){
					return;
				}

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



