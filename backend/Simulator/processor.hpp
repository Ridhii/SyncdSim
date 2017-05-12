#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP


#include "common.hpp"
#include "context.hpp"

class Context;

class Processor{

private:
	contech::Task* currTask;
	std::vector<contech::MemoryAction> memActionQueue;
	Context* myContext;
	int tempTotalTask;

public:
	Processor(Context* context);
	void run();
	void populateMemActionQueue();
	void reAddCurrMemOp(uint64_t addr, contech::action_type type);


};


#endif

