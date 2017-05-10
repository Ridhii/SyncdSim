#include "directory.hpp"
protocolType protocol;

Directory::Directory(Context* context) {
	
	myContext = context;
	numContexts = myContext->getNumContexts();
	
}

Directory::~Directory() {}

DirectoryEntry& Directory::lookUpEntry(uint64_t addr) {
	if(directory.find(addr) == directory.end()){
		//printf("no entry in directory, creating now\n");
		updateEntry(addr, DirectoryEntryStatus::UNCACHED, -1);
	}
	//printf("returning from directory \n");
	return directory[addr];
}


void Directory::updateEntry(uint64_t addr, DirectoryEntryStatus status, int procID) {
	
	if(directory.find(addr) == directory.end()){
		DirectoryEntry dirEntry;
		dirEntry.processorMask.reserve(numContexts);
		for(int i = 0; i < numContexts; i++){
			if(i == procID){
				dirEntry.processorMask.push_back(true);
			}
			else{
				dirEntry.processorMask.push_back(false);
			}
		}
		dirEntry.status = status;
		directory.insert(std::pair<uint64_t, DirectoryEntry>(addr, dirEntry));
		return;
	}
	
	if(status == DirectoryEntryStatus::SHARED){
		directory[addr].status = status;
		directory[addr].processorMask[procID] = true;
	}
	
	if(status == DirectoryEntryStatus::EXCLUSIVE){
		assert(protocol == protocolType::MESI);
		assert(directory[addr].status == DirectoryEntryStatus::UNCACHED);
		directory[addr].status = status;
		for(int i = 0; i < numContexts; i++){
			if(i == procID){
				directory[addr].processorMask[i] = true;
			}
			else{
				directory[addr].processorMask[i] = false;
			}
		}

	}

	if(status == DirectoryEntryStatus::MODIFIED){
		directory[addr].status = status;
		for(int i = 0; i < numContexts; i++){
			if(i == procID){
				directory[addr].processorMask[i] = true;
			}
			else{
				directory[addr].processorMask[i] = false;
			}
		}
	}

	if(status == DirectoryEntryStatus::UNCACHED){
		directory.erase(addr);
	}
}
	
DirectoryEntry& Directory::getOwners(uint64_t addr){
	assert(directory.find(addr) != directory.end());
	return directory[addr];

}
