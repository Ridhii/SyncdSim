#include "common.hpp"
#include "context.hpp"
#include "assert.h"
#include <map>


Directory::Direcotry(Context* context) {
	
	myContext = context;
	numContexts = myContext->getNumContexts();
	
}

Directory::~Direcotry() {}

DirectoryEntryStatus Directory::lookUpEntry(uint64_t addr) {
	if(directory.find(addr) == directory.end()){
		return DirectoryEntryStatus.UNCACHED;
	}
	else{
		return directory.find(addr).status;
	}

}


void Directory::addEntry(unint64 addr, DirectoryEntryStatus status, int procID) {
	
	if(directory.find(addr) == directory.end()){
		DirectoryEntry* dirEntry = new DirectoryEntry;
		dirEntry->processorMask.reserve(numContexts);
		for(int i = 0; i < numContexts; i++){
			if(i == procID){
				dirEntry->processorMask.push_back(true);
			}
			else{
				dirEntry->processorMask.push_back(false);
			}
		}
		dirEntry->status = status;
		directory[addr] = *(dirEntry);
		return;
	}
	if(status == DirectoryEntryStatus.SHARED){
		directory[addr].status = status;
		directory[addr].processorMask[procID] = true;
	}
	if(status == DirectoryEntryStatus.MODIFIED){
		directory[addr].status = status;
		for(int i = 0; i < numContexts; i++){
			if(i == procID){
				directory[addr].processorMask[procID] = true;
			}
			else{
				directory[addr].processorMask[procID] = false;
			}
		}

	}
	if(status == DirectoryEntryStatus.UNCACHED){
		directory.erase(addr);

	}
}

DirectoryEntry getOwners(uint64_t addr){
	assert(directory.find(addr) != directory.end());
	return directory[addr];

}

void Directory::removeEntry(uint64_t addr) {


}