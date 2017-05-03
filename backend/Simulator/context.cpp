#include "context.hpp"


Context::Context(int _contextID, protocolType _protocol, int _numContexts){
	
	contextID = _contextID;
	numContexts = _numContexts;

	// create protocolHandler object based on the protocol
	switch(_protocol) {
    case MSI: 
    	pH = new MSIProtocolHandler(this);
    	break;
    default:
    	pH = new MSIProtocolHandler(this);
	}

	// create processor object
	processor = new Processor(this);

	// create directory object
	dir = new Directory(this);

	// create cache object
	cache = new Cache(this);

}


void Context::run(){
      pH -> checkIncomingMsgs();
      processor -> run();
}


void Context::addToTaskQueue(Task* _task){
	taskQueue.push(_task);
}

Task* Context::getNextTask(){
	if (taskQueue.empty())	return NULL;
	Task* nextTask = taskQueue.front();
	taskQueue.pop();
	return nextTask;
}

void Context::setMemOp(uint64 _addr, action_type _type) {
	currentMemOp.address = _addr;
	currentMemOp.action_type = _type;
}

MemOp& Context::getMemOp() {
	return currentMemOp;
}

void Context::addCompletedTask(Task* _task) {
	completedTasks.emplace_back(_task);
}

vector<Task*>& Context::getCompletedTasks() {
	return completedTasks;
}

void Context::clearCompletedTasks() {
	completedTasks.clear();
}

void Context::setSuccessful(bool isSuccessful) {
	currentMemOpSuccessful = isSuccessful;
}

bool Context::getSuccessful() {
	return currentMemOpSuccessful;
}

int Context::getNumContexts(){
	return numContexts;
}

void addCacheMsg(Message* msg) {
	cacheMsgQueue.emplace_back(msg);
}

vector<Message>& getCacheMsgQueue() {
	return cacheMsgQueue;
}

void addToIncomingMsgQueue(Message* msg) {
	incomingMsgQueue.emplace_back(msg);
}

vector<Message>& getIncomingMsgQueue() {
	return incomingMsgQueue;
}




