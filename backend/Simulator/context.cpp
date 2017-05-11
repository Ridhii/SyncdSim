#include "context.hpp"


Context::Context(int _contextId, protocolType _protocol, Simulator* _simulator){
	contextId = _contextId;
	simulator = _simulator;
	// create protocolHandler object based on the protocol
	switch(_protocol) {
    case MSI: 
    	pH = new MSIHandler(this);
    	break;
    case MESI:
    	pH = new MESIHandler(this);
    	break;
    default:
    	pH = new MSIHandler(this);
	}

	// create processor object
	processor = new Processor(this);

	// create directory object
	directory = new Directory(this);

	// create cache object
	cache = new Cache(this);
	currentMemOpSuccessful = true;

	cacheHit = 0;
	cacheMiss = 0;
	numSentMsgs = 0;
	numSentMsgsToCache = 0;
	numInvalidationsSent = 0;
	EStateCount = 0;


}


void Context::run(){
	if(numTasksLeft == 13) cout << "context " << contextId << "starts\n";
	clearCompletedTasks();
	cache -> run();
    pH -> checkIncomingMsgQueue();
    processor -> run();
    if(numTasksLeft == 13) cout << "context " << contextId << "finishes\n";
}


void Context::addToTaskQueue(contech::Task* _Task){
	taskQueue.push(_Task);
}

contech::Task* Context::getNextTask(){
	if (taskQueue.empty())	return NULL;
	contech::Task* nextTask = taskQueue.front();
	taskQueue.pop();
	return nextTask;
}

void Context::setMemOp(uint64_t addr, contech::action_type type) {
	currentMemOp.addr = addr;
	currentMemOp.actionType = type;
}

MemOp& Context::getMemOp() {
	return currentMemOp;
}

void Context::addCompletedTask(contech::Task* Task) {
	completedTasks.push_back(Task);
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
	cacheMsgQueue.push_back(msg);
}

std::vector<Message*>& Context::getCacheMsgQueue() {
	return cacheMsgQueue;
}

void Context::addToIncomingMsgQueue(Message* msg) {
	//printf("MSG IS ADDED %d'S QUEUE\n", contextId);
	incomingMsgQueue.push_back(msg);
}

std::vector<Message*>& Context::getIncomingMsgQueue() {
	return incomingMsgQueue;
}

int Context::getContextId() {
	return contextId;
}

int Context::getHomeNodeIdByAddr(uint64_t addr) {
	int numContexts = simulator -> getNumContexts();
	//int shift = 64 - (int)log2(numContexts);
	/* 64 bit addresses are truncated to 48 bits */
	int shift = 48 - (int)log2(numContexts);

	return addr >> shift;
}

Context* Context::getContextById(int id) {
	return simulator -> getContextById(id);
}

int Context::getNumContexts() {
	return simulator -> getNumContexts();
}

DirectoryEntry& Context::lookupDirectoryEntry(uint64_t addr) {
	return directory -> lookUpEntry(addr);
}

void Context::updateDirectoryEntry(uint64_t addr, DirectoryEntryStatus status, int pid) {
	directory -> updateEntry(addr, status, pid);
}

void Context::handleMemOpRequest() {
	pH -> handleMemOpRequest();
}

void Context::incCacheHit(){
	cacheHit += 1;
}

void Context::incCacheMiss(){
	cacheMiss += 1;
}

void Context::incNumSentMsgs(){
	numSentMsgs += 1;

}

void Context::incNumInvalidations(){
	numInvalidationsSent += 1;

}

void Context::incNumSentMsgsToCache(){
	numSentMsgsToCache += 1;

}

void Context::incEStateCount(){
	EStateCount += 1;

}

void Context::printContextStats(){
	printf("***** CONTEXT ID %d *****\n", contextId);
	printf(" CacheHits = %d CacheMisses = %d, numSentMsgs = %d, numInvalidationsSent = %d numSentMsgsToCache = %d\n", cacheHit, cacheMiss, numSentMsgs, numInvalidationsSent, numSentMsgsToCache);
	if(protocol == protocolType::MESI){
		printf("EStateCount = %d\n", EStateCount);
	}

}









