#include "../../common/taskLib/Action.hpp"
#include "../../common/taskLib/Task.hpp"
#include "context.hpp"
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <vector>
#include <map>
#include <algorithm>


#define ALIGNER 0xFFFFFE00
#define LINE_SIZE 64
#define ACTION_SIZE 8
#define POW_SIZE 3



class Processor{

private:
	Task* currTask;
	std::queue<MemoryAction> memActionQueue;
	Context* myContext;

public:
	Processor(Context* context);
	void run();
	void populateMemActionQueue();


};

