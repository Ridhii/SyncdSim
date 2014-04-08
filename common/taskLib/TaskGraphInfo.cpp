#include "TaskGraphInfo.hpp"

using namespace contech;

void TaskGraphInfo::initTaskGraphInfo(ct_file* in)
{
    int numBasicBlock = 0;
    
    ct_read(&numBasicBlock, sizeof(int), in);
    for (int i = 0; i < numBasicBlock; i++)
    {  
        uint strLen;
        uint bbid = 0;
        uint lineNumber, numOfMemOps;
        char* f;
        string function, file;
        
        ct_read(&bbid, sizeof(uint), in);
        ct_read(&lineNumber, sizeof(uint), in);
        ct_read(&numOfMemOps, sizeof(uint), in);
        
        ct_read(&strLen, sizeof(uint), in);
        if (strLen > 0)
        {
            f = (char*) malloc(sizeof(char) * strLen);
            ct_read(f, sizeof(char) * strLen, in);
            function.assign(f);
            free(f);
        }
        
        ct_read(&strLen, sizeof(uint), in);
        if (strLen > 0)
        {
            f = (char*) malloc(sizeof(char) * strLen);
            ct_read(f, sizeof(char) * strLen, in);
            file.assign(f);
            free(f);
        }
        
        addRawBasicBlockInfo(bbid, lineNumber, numOfMemOps, function, file);
    }
}

TaskGraphInfo::TaskGraphInfo()
{

}

void TaskGraphInfo::addRawBasicBlockInfo(uint bbid, uint lineNum, uint numMemOps, string function, string file)
{
    BasicBlockInfo bbi;
    
    bbi.lineNumber = lineNum;
    bbi.numOfMemOps = numMemOps;
    bbi.functionName = function;
    bbi.fileName = file;
    
    bbInfo[bbid] = bbi;
}

void TaskGraphInfo::writeTaskGraphInfo(ct_file* out)
{
    int numBasicBlock = bbInfo.size();
    ct_write(&numBasicBlock, sizeof(int), out);
    for (auto it = bbInfo.begin(), et = bbInfo.end(); it != et; ++it)
    {
        uint strLen;
        uint bbid = it->first;
        ct_write(&bbid, sizeof(uint), out);
        ct_write(&it->second.lineNumber, sizeof(uint), out);
        ct_write(&it->second.numOfMemOps, sizeof(uint), out);
        
        strLen = it->second.functionName.length();
        ct_write(&strLen, sizeof(uint), out);
        ct_write(it->second.functionName.c_str(), sizeof(char) * strLen, out);
        
        strLen = it->second.fileName.length();
        ct_write(&strLen, sizeof(uint), out);
        ct_write(it->second.fileName.c_str(), sizeof(char) * strLen, out);
    }
}

BasicBlockInfo& TaskGraphInfo::getBasicBlockInfo(uint bbid)
{
    BasicBlockInfo bbi;
    auto it = bbInfo.find(bbid);
    //if (it == bbInfo.end()) return bbi;
    return it->second;
}
