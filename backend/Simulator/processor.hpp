#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP


#include "common.hpp"
#include "context.hpp"

class Context;

class Processor{

private:
	contech::Task* currTask;
	std::queue<contech::MemoryAction> memActionQueue;
	Context* myContext;

public:
	Processor(Context* context);
	void run();
	void populateMemActionQueue();


};


#endif

