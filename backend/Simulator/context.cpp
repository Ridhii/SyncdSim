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
      pH -> checkIncomingMsgQueue();
      processor -> run();
}


void Context::addToTaskQueue(contech::Task* _Task){
	taskQueue.push(_Task);
}

contech::Task* Context::getNextTask(){
	if (contech::TaskQueue.empty())	return NULL;
	contech::Task* nextTask = taskQueue.front();
	taskQueue.pop();
	return nextTask;
}

void Context::setMemOp(uint64 addr, contech::action_type type) {
	currentMemOp.addr = addr;
	currentMemOp.action_type = type;
}

MemOp& Context::getMemOp() {
	return currentMemOp;
}

void Context::addCompletedTask(contech::Task* Task) {
	completedTasks.emplace_back(Task);
}

vector<contech::Task*>& Context::getCompletedTasks() {
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

std::map<uint64_t, std::queue<Message*> >& Context::getBlockedMsgMap() {
	return blockedMsgMap;
}

int Context::getNumContexts() {
	return simulator -> getNumContexts();
}

DirectoryEntry Context::lookupDirectoryEntry(uint64_t addr) {
	return directory -> lookupEntry(uint64_t);
}

void Context::updateDirectoryEntry(uint64_t addr, DirectoryEntryStatus status, int pid) {
	directory -> updateEntry(addr, status, pid);
}









