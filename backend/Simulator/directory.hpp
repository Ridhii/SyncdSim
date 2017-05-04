#ifndef DIRECTORY_H
#define DIRECTORY_H

enum DirectoryEntryStatus {UNCACHED, SHARED, MODIFIED}  // by other processors

struct DirectoryEntry
{
	// a bit vector where bit i indicates whether Pi currently owns the line
	std::vector<bool> processorMask;	
	DirectoryEntryStatus status;
};


class Direcotry
{
private:
	Context* myContext;
	int numContexts;
	std::map<uint64_t addr, DirectoryEntry> directory;

public:
	Direcotry(Context* context);
	~Direcotry();

	DirectoryEntryStatus lookUpEntry(uint64_t addr);
	void addEntry(uint64_t addr, DirectoryEntryStatus status, int procID);
	DirectoryEntry getOwners(uint64_t addr);
	void removeEntry(unint64 addr);
};


#endif