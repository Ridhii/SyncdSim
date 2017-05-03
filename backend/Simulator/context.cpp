#include "context.hpp"


Context::Context(int _contextId, protocolType _protocol, Simulator* _simulator){
	contextId = _contextId;
	simulator = _simulator;
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

void Context::setMemOp(uint64 addr, action_type type) {
	currentMemOp.addr = addr;
	currentMemOp.action_type = type;
}

MemOp& Context::getMemOp() {
	return currentMemOp;
}

void Context::addCompletedTask(Task* task) {
	completedTasks.emplace_back(task);
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

void Context::addToCacheMsgQueue(Message* msg) {
	cacheMsgQueue.emplace_back(msg);
}

vector<Message*>& Context::getCacheMsgQueue() {
	return cacheMsgQueue;
}

void Context::addToIncomingMsgQueue(Message* msg) {
	incomingMsgQueue.emplace_back(msg);
}

vector<Message*>& Context::getIncomingMsgQueue() {
	return incomingMsgQueue;
}

int Context::getContextId() {
	return contextId;
}

int Context::getHomeNodeIdByAddr(uint64_t addr) {
	// TODO!!!
	return -1;
}

Context* Context::getContextById(int id) {
	return simulator -> getContextById(id);
}



