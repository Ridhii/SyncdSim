#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "common.hpp"
#include "context.hpp"


enum DirectoryEntryStatus {UNCACHED, SHARED, MODIFIED}  // by other processors

struct DirectoryEntry
{
	// aligned address
	unint64 addr;	
	// a bit vector where bit i indicates whether Pi currently owns the line
	std::vector<bool> processorMask;	
	DirectoryEntryStatus status;
};


class Direcotry
{
private:
	Context* myContext;

	// TODO - need some data structures

public:
	Direcotry(Context* context);
	~Direcotry();

	LineStatus lookUpEntry(unint64 addr);
	void addEntry(unint64 addr, LineStatus status);
	void removeEntry(unint64 addr);
};


#endif