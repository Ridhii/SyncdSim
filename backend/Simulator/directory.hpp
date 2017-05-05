#ifndef DIRECTORY_H
#define DIRECTORY_H
#include "common.hpp"

class Context;

enum DirectoryEntryStatus {UNCACHED, SHARED, MODIFIED};  // by other processors

struct DirectoryEntry
{
	// a bit vector where bit i indicates whether Pi currently owns the line
	std::vector<bool> processorMask;	
	DirectoryEntryStatus status;
};

class Directory
{
private:
	Context* myContext;
	int numContexts;
	std::map<uint64_t, DirectoryEntry> directory;

public:

	Directory(Context* context);
	~Directory();

	DirectoryEntry& lookUpEntry(uint64_t addr);
	void updateEntry(uint64_t addr, DirectoryEntryStatus status, int procID);
	DirectoryEntry& getOwners(uint64_t addr);
};


#endif