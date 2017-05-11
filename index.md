# Overview
Syncdsim is a directory-based cache coherence simulator that supports MSI and MESI (more to come). It takes in memory reference traces, simulates cache and directory traffic, and finally analyzes/reports the behaviors. 

# Background
Cache coherence is one of the most important topics in designing multi-processor caches. In the lectures, we discussed both snooping-based and directory-based cache coherence protocols. Comparing to snooping-based which relies heavily on broadcasting on the entire bus, directory-based protocols seems to be more scalable with regard to number of processors as it allows point-to-point communication. Therefore, we decide to develop a deeper understanding of the various directory-based cache coherence protocols by actually implementing them and observe cache behavior of programs with distinct memory traces. We hope that our project would eventually come available as a tool for programmers who are interested in knowing the cache behavior and memory reference characteristics of their programs, which could potentially be helpful in optimizing the code. 

# Design
## Simulator
## Context
## Processor
## Cache
## Directory
## Protocol Handler
### MSI Handler
### MESI Handler

# Correctnest Test
## Basic correctness
We tested the correctness for both MSI and MESI by creating a small sequence of memory operations for a number of processors. We sketched out the behaviors of each component as well as their communication that we expect to observe during each cycle, and then carefully verifies the result with our simulator output. 

Since our project is designed to take in a taskGraph file and parse the taskGraph for assigning tasks(groups of mem ops) to each processor, we had to do a bit of hacking to "inject" our mem ops into an existing taskGraph, that is to replace the original mem ops in the graph with our own ops. 

This approach is useful in getting some basic correctness verified, but it's extremely non-scalable as the complexity grows fast with number of mem ops.

The example mem op sequence and expected behavior of simulator for MESI can be found [here (https://docs.google.com/document/d/1DDc1RicXqDVquXbJmqvOqBLhzjSF1iNKCU6ISPp7JIg/edit)] 

The example mem op sequence and expected behavior of simulator for MESI can be found [here](https://docs.google.com/document/d/1j2hKFtNprdb43laDmoDHZk0d-TV_UeslHpxe_8zuYuM/edit?usp=sharing)


# Analysis
For the analysis we ran a small taskGraph with memOps per task capped at 5 (to be able to finish quickly). However, the analyzation can be extended to any number of memOps per task. Here we see that cacheHits and cacheMisses for both MSI and MESI remain same as irrespective of the fact that a line is found in an exclusive state or modified state in MESI during a write it's still a cacheHit. The other conditions of cacheHit remain same across MSI and MESI and hence this count is same.
Same argument is applied to cacheMiss. Here comes the interesting part - EStateCount represents the number of times a line is found in exclusive state. That is 1 for this file in MESI count and subsequently numInvalidations sent out is 1 less in MESI as the node does not send out invalidates for a write to exclusive state. Whereas the same line was in a Modified state in MSI and an invalidate was sent out for it. Number of messages sent to other nodes by a node is more in MESI protocol as a home node sends out a FETCH to a node containing a line in E state when another node sends a READ_MISS as the node holding the line in E state might have written to it. However, when a node sends a READ_MISS for a line that is in Shared state, the home node does not send any messages to any of the sharers as the line has not been modified since last written. This is not guaranteed when a node holds a line in E state in MESI (it might have migrated to M). As for the messages sent to cache, that increases in MESI because we send out a CACHE_FETCH when we get a request to FETCH a line that is held in an E state in MESI from the home node of that line upon receiving a READ_MISS from another node.

![alt](backend/Simulator/Images/blackscholes.png)

More comparisons to follow!

TestAndSet vs Test-TestAndSet - To be able to compare the results from test-testAndSet and testAndSet in MESI protocol to see how does the protocolHandler react to different types of reads and writes (in tas we issue a read-modify-write causing invalidations while in ttas we issue a read until invalidated by the owner) we wrote two small cpp files and professor Railing very generously provided us with taskGraphs for those. We are in the process of analysing and generating results from those!




# Conclusion
