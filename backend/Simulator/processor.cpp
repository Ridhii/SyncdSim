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

	/*==============================BASIC TEST HACK==============================*/

	// assert(currTask != NULL);
	// printf("in populateMemActionQueue \n");
	// /* only called to populate the memActionQueue with the 
	//    currTask's memOps
	// */
	// contech::Task::memOpCollection memOps = currTask->getMemOps();
	// auto iter = memOps.begin();
	// /* FOR DEBUGGING */
	// int myID = myContext->getContextId();
	// uint64_t A0, A1, A2, A3;
	// A0 = 0x0bcdabcdabcd;
	// A1 = 0x4bcdabcdabcd;
	// A2 = 0x8bcdabcdabcd;
	// A3 = 0xcbcdabcdabcd;
	// /*
	// C0: read A2, write A2
	// C1: read A0, write A0
	// C2: read A0
	// C3: write A3
	// */
	// if (myID == 0) {
	// 	contech::MemoryAction action1{(uint64_t)A2,
 //        		                       (uint64_t)POW_SIZE, 
 //        		                       (contech::action_type)(contech::action_type_mem_read)};
 //     	contech::MemoryAction action2{(uint64_t)A2,
 //        		                       (uint64_t)POW_SIZE, 
 //        		                       (contech::action_type)(contech::action_type_mem_write)};
	// 	memActionQueue.push(action1);
	// 	memActionQueue.push(action2);
	// }
	// else if (myID == 1) {
	// 	contech::MemoryAction action1{(uint64_t)A0,
 //        		                       (uint64_t)POW_SIZE, 
 //        		                       (contech::action_type)(contech::action_type_mem_read)};
 //     	contech::MemoryAction action2{(uint64_t)A0,
 //        		                       (uint64_t)POW_SIZE, 
 //        		                       (contech::action_type)(contech::action_type_mem_write)};
	// 	memActionQueue.push(action1);
	// 	memActionQueue.push(action2);
	// }
	// else if (myID == 2) {
	// 	contech::MemoryAction action1{(uint64_t)A0,
 //        		                       (uint64_t)POW_SIZE, 
 //        		                       (contech::action_type)(contech::action_type_mem_read)};
 //     	memActionQueue.push(action1);
	// }
	// else {
	// 	contech::MemoryAction action1{(uint64_t)A3,
 //        		                       (uint64_t)POW_SIZE, 
 //        		                       (contech::action_type)(contech::action_type_mem_write)};
 //     	memActionQueue.push(action1);
	// }
	
}

void Processor::run(){
	cout << "mem op queue size is " << memActionQueue.size() << endl;
	while(myContext->getSuccessful()){
		//if(numTasksLeft == 39) cout << "entering the while loop in processor.run()\n";
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
				//cout << " ******** CONTEXT " << myContext->getContextId() << " FINISHES IT'S TASK ********\n";
				myContext->addCompletedTask(currTask);
				currTask = NULL;

			}
			//printf("getting the next task \n");
			currTask = myContext->getNextTask();
			myContext->setSuccessful(true);
			if(currTask != NULL){
				populateMemActionQueue();
				//if(numTasksLeft == 39) cout << "populateMemActionQueue returns \n";
				taskStarted += 1;
				//cout << " ******** TASK " << taskStarted << " STARTS IN CONTEXT" << myContext->getContextId() << " ******** \n";
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



