#ifndef CONTECH_H
#define CONTECH_H

// These are included in contech.cpp, which creates the appropriate path
//  given that llvm decided to put the headers in different directories
//#include "llvm/Pass.h"
//#include "llvm/Module.h"

#include <string>
#include <ct_event_st.h>

namespace llvm {
    class Contech;
    ModulePass* createContechPass();
    
    typedef struct _llvm_mem_op {
        bool isWrite;
        bool isGlobal;
        char size;
        Value* addr;
        struct _llvm_mem_op* next;
    } llvm_mem_op, *pllvm_mem_op;

    typedef struct _llvm_basic_block {
        unsigned int id, len, lineNum, numIROps, critPathLen;
        int hasCheckBuffer;
        bool containCall;
        bool containGlobalAccess;
        ct_event_id ev; // if ev == ct_event_basic_block, then no sync in this block
        pllvm_mem_op first_op;
        std::string fnName;
        //const char* fnName;
        const char* fileName;
        BasicBlock* tgts[2]; // basic blocks may branch into up to two other blocks
    } llvm_basic_block, *pllvm_basic_block;
    
    typedef enum _CONTECH_FUNCTION_TYPE {
        NONE,
        MAIN,
        MALLOC,
        MALLOC2, // Calls like memalign(align, size)
        FREE,
        THREAD_CREATE,
        THREAD_JOIN,
        SYNC_ACQUIRE,
        SYNC_RELEASE,
        BARRIER,
        BARRIER_WAIT,
        EXIT,
        COND_WAIT,
        COND_SIGNAL,
        OMP_CALL,
        OMP_FORK,
        OMP_FOR_ITER,
        OMP_BARRIER,
        OMP_END,
        GLOBAL_SYNC_ACQUIRE, // Syncs that have no explicit address
        GLOBAL_SYNC_RELEASE,
        NUM_CONTECH_FUNCTION_TYPES
    } CONTECH_FUNCTION_TYPE;
    
    typedef struct _llvm_function_map {
        const char* func_name;
        size_t str_len;
        CONTECH_FUNCTION_TYPE typeID;
    } llvm_function_map, *pllvm_function_map;
}

#endif