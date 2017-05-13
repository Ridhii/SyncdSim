# Overview
Syncdsim is a directory-based cache coherence simulator that supports MSI and MESI (more to come). It takes in memory reference traces, simulates cache and directory traffic, and finally analyzes/reports the behaviors. 

# Background
## Motivation
Cache coherence is one of the most important topics in designing multi-processor caches. In the lectures, we discussed both snooping-based and directory-based cache coherence protocols. Comparing to snooping-based which relies heavily on broadcasting on the entire bus, directory-based protocols seems to be more scalable with regard to number of processors as it allows point-to-point communication. Therefore, we decide to develop a deeper understanding of the various directory-based cache coherence protocols by actually implementing them and observe cache behavior of programs with distinct memory traces. We hope that our project would eventually come available as a tool for programmers who are interested in knowing the cache behavior and memory reference characteristics of their programs, which could potentially be helpful in optimizing the code. 

# Design
![alt](https://github.com/Ridhii/SyncdSim/blob/master/backend/Simulator/architecture.png)
![alt](https://github.com/Ridhii/SyncdSim/blob/master/backend/Simulator/message_sequence.png)
## Simulator
## Context

## Processor
Processor is responsible for getting memory operations from a Task. It keeps a queue of MemoryAction objects that contain an address and an associated memory action type (e.g. a action_type_mem_read), and feeds the memory actions to protocol handler one at a time, and only when the previous memory action has finished. When the queue runs empty, the processor tries to fetch a new Task if one is available, and pre-processes the address of a memory action to be 64-byte aligned (which is the size of a cache line). In the case when a memory address spans two lines, processor will split that Memory Action into two, that have identical memory action types.

## Cache
Our Cache supports configurable number of sets as well as associativity. Cache keeps a message queue and all messages are from local Protocol Handler, requesting to read a line, update (add/modify) a line, or invalidate a line. Cache will take request one by one, fulfill it, and ACK back to the Protocol Handler by adding to its incoming message queue. When eviction happens during an update, the Cache uses LRU policy to select a victim, and notify the Protocol Handler who could then change the cache line status accordingly.


## Directory

## Protocol Handler
Protocol Handler knows well about the protocol and its state transitions. It talks to Directory, Cache, and other nodes in order to service a MemoryAction from its own processor, or a message from its incoming message queue. 

1. Service a MemoryAction from its own processor

Protocol Handler maintain a map that stores the current state of a line in its own cache. The state could be "MODIFIED", "SHARED", or "INVALID" if the protocol is MSI, with an extra "EXCLUSIVE" state in MESI. To service a MemoryAction, it checks to see the current state of the line, and decides whether it could just read from/write to its own Cache, or it actually has to communicate with the home node. If latter, it will choose the message type based on the current protocol being used. For cache-related messages, Protocol Handler adds the message to the message queue of local cache. For other messages, it puts the message on other Context's incoming message queue. As an example, in *MSI*, the correct message type for all combinations would look like:

| Cache Line Status | Memory Action | Msg Type         | Description                                       |
|-------------------|---------------|------------------|---------------------------------------------------|
| MODIFIED          | READ          | CACHE_READ       | read from local cache                             |
| MODIFIED          | WRITE         | CACHE_UPDATE     | write to local cache                              |
| SHARED            | READ          | CACHE_READ       | read from local cache                             |
| SHARED            | WRITE         | INVALIDATE_OTHER | ask the home node to invalidate all other sharers |
| INVALID           | READ          | READ_MISS        | ask the home node for a line to read              |
| INVALID           | WRITE         | WRITE_MISS       | ask the home node for a line to write             |


2. Service a message from its incoming message queue

Each cycle, a Protocol Handler will check its incoming queue for serviceable messages, and reacts accordingly. If the message has type INVALIDATE_OTHER, READ_MISS, or WRITE_MISS, then it's possible that another message with regard to the same memory address is currently being serviced. In that case, the later message could not be serviced right away, hence will be put in to a blocked message queue for that address. If the message is serviceable, Protocol Handler will check whether it could reply immediately, or it needs to relay it to owners/sharers of the line.

Another category of messages are those from home node that asks the protocol Handler to "do something" about a particular cache line. The message type could be INVALIDATE (S -> I), FETCH_INVALIDATE (M -> I), or just FETCH (M -> S). It's also possible that home node has sent an ACK for a line we previously requested. Upon getting those messages, Protocol Handler will modify the cache line status for this line, and send out a message to its own cache.

Finally, Protocol Handler could be getting messages from its local cache, in most cases an ACK for a cache action. For those messages, Protocol Handler either replies to home node with an ACK, or completes its current Memory Action (which means Processor could go ahead and fetch the next one), based on the specific type of that ACK message. 


# Correctnest Test
We tested the correctness for both MSI and MESI by creating a small sequence of memory operations for a number of processors. We sketched out the behaviors of each component as well as their communication that we expect to observe during each cycle, and then carefully verifies the result with our simulator output. 

Since our project is designed to take in a taskGraph file and parse the taskGraph for assigning tasks(groups of mem ops) to each processor, we had to do a bit of hacking to "inject" our mem ops into an existing taskGraph, that is to replace the original mem ops in the graph with our own ops. 

The example mem op sequence and expected behavior of simulator for MESI can be found [here]( https://docs.google.com/a/andrew.cmu.edu/document/d/1DDc1RicXqDVquXbJmqvOqBLhzjSF1iNKCU6ISPp7JIg/edit?usp=sharing)

The example mem op sequence and expected behavior of simulator for MSI can be found [here]( https://docs.google.com/document/d/1j2hKFtNprdb43laDmoDHZk0d-TV_UeslHpxe_8zuYuM/edit?usp=sharing)

This approach is useful in getting some basic correctness verified, but it's extremely non-scalable as the complexity grows fast with number of Memory Actions and number of Tasks. Therefore, we also used a lot of asserts to help us verify the state transitions of cache and directory entries are as expected. We have been able to run our simulation on some fairly large TaskGraphs without failing, and in the following section we will discuss some of our observations from running those traces.


# Analysis
## TestAndSet vs Test-TestAndSet 

The following chart represents information about the statistics we collected for taskGraphs that represented programs we wrote that represented test and set and test - test and set schemes.
![alt](https://github.com/Ridhii/SyncdSim/blob/master/backend/Simulator/Images/tasVsttas.png)

Clarification on the x-axis labels : 

#msg sent to other nodes - represents the number of times a context sent a message to another context to request a certain service (like service a READ_MISS etc). 
#write to E represents the total number of times the contexts wrote to a line in an Exclusive state and #write to M represents the total number of times all the contexts wrote to a line in a Modified state.
#invalidations sent means the total number of times all the contexts wrote to a line in a shared state and sent the home node an invalidation request.

The simulator takes in the taskgraphs generated for the test and set and test-test and set programs and simulates the memory operations in each task in a task dependent order. The following are the taskgraphs for test and set and test-test and set and will help understand the above graph better. 

Cache Hits and Misses - Looking at tas.png we see that test and set is sequential in the middle. Context 3 loads the line in a shared state and invalidates it when trying to write to it. After that no context contends for that line for a while and hence context 3 has a cache hit everytime it tries to access the line during it’s sequential access. Same behavior is seen with context 0 and hence both of them have a large number of cache hits. The sequential nature of the taskgraph also explains why test and set has more cache hits than cache misses.
The ttas.png suggests that the number of tasks in test-test and set is much lesser than that in test and set. This combined with the fact the test-test and set is much more parallel suggests that contexts in test - test and set setting have lesser cache hits. Since, all the contexts execute in parallel most of the program time, a context will likely not have a line in a shared/modified/exclusive state more than a few clock cycles and hence incurs more misses than hits.

Number of messages sent to other nodes/contexts - The test and set taskgraph has a larger number of messages sent to other nodes because of two reasons - first it has a larger number of tasks and hence has more memory operations that lead to more messages sent across the interconnect. Secondly, because test and set reads directly from the memory, it requires more communication to invalidate/fetch/fetch-invalidate the line than test - test and set.
Test - test and set has a lesser number of messages sent across the interconnect as it has a  fewer number of tasks and hence sends out a lesser number of messages and secondly, since tasks in test - test and set only issue invalidations upon unlocking and not while reading a line in a shared state in their cache.

Number of invalidations sent - Both test and set and test - test and set send out invalidations, however number is more in test and set because of the higher number of memory operations and read-modify-write intentions issued which result in invalidation of the line in other contexts.

Number of writes in an Exclusive state : When test and set and test - test and set are simulated with MESI protocol, contexts do load a line in an exclusive state however because of the contentious nature of the program, that line is brought down to a shared state by a second context trying to read from it too, and hence neither of the program have the benefit of writing to a line in an exclusive state. However, as you will see with taskgraphs of larger sizes with less contention, a line loaded in an exclusive state can be written to without incurring the overhead of communicating with other contexts.

Number of writes in a modified state : For test and set, context 0 and context 3 execute sequentially for a good portion of program time and hence keep writing to a line in a modified state in their cache without any contention and hence the number is high for test and set. Infact, it’s the write - modified hit count that is a major contributor to the the total cache hits of test and set.
Test - test and set has zero writes to a line in a modified state as the contexts either read to a line in a shared state and when the lock holder unlocks the lock and invalidates the line, the next holder of the line gets the line in a modified state and immediately writes to it and finishes the task. Thus, the contexts in test - test and set never issue an intention to write and fail in doing so - they either read the line in a shared state or when the line is invalidated, load and write to it and then finish while the rest of the contexts have the line in an invalid state.


## Splash
Below is a total count for differernt stats collected from running Splash taskgraph with MSI and MESI:

|                          | MSI    | MESI   |
|--------------------------|--------|--------|
| Total Cycle              | 304724 | 304434 |
| Cache Hit                | 294740 | 294740 |
| Cache Miss               | 4597   | 4597   |
| Context Messages         | 11623  | 13775  |
| Cache Messages           | 4885   | 4597   |
| Invalidation Messages    | 288    | 0      |
| Write to line in E state | 0      | 288    |
| Write to line in M state | 123056 | 123056 |


Note that, "Invalidation Messages" here refer to number of "INVALIDATE_OTHER" messages, and that is the message a node would send to home node if it would like to write to a line and figures that the current cache status is SHARED (hence it requests the home node to invalidate everyone else who has the line).

We can see that with MESI state, there is 0 invalidation messages, which makes perfect sense because in MESI, Protocol Handler will simply write to the line in "EXCLUSIVE" state and promote itself to "MODIFIED", without the need to notify the home node. However, there's an increase in total number of messages sent among contexts. An explanation would be that, the home node will now have to treat an "EXCLUSIVE" line as if it's "MODIFIED" because it has no knowledge whether the owner has written to it or not. Therefore in MESI, the home node ends up sending INVALIDATE_FETCH to those exclusive owners when it gets a READ_MISS from someone, while in MSI when a line is in "SHARED" state, the home node can directly reply to a READ_MISS without invalidating the sharers.



## BlackScholes
Below is a total count for differernt stats collected from running BlackScholes taskgraph with MSI and MESI:

|                          | MSI    | MESI   |
|--------------------------|--------|--------|
| Total Cycle              | 62998  | 62997  |
| Cache Hit                | 165042 | 165042 |
| Cache Miss               | 30001  | 30001  |
| Context Messages         | 60076  | 88602  |
| Cache Messages           | 30011  | 30010  |
| Invalidation Messages    | 3      | 2      |
| Write to line in E state | 0      | 1      |
| Write to line in M state | 10680  | 10680  |

For this task graph, there really isn't that much of difference in simulation result whether running with MSI or MESI. The fact that MESI is not particularly useful for this task graph suggests some characteristics about it. Specifically, the task graph must be:

+ having very few READ followed by WRITE. Otherwise, since the first reader enters EXCLUSIVE, its subsequent WRITEs to the line will be write hits.  Or, 

+ having a lot of contending readers, which implies that the line is most likely be in SHARED state rather than owned by someone exclusively



# Reference
[Contech Refernce Framework](https://github.com/bprail/contech) 

John L. Hennessy, David A. Patterson *Computer Architecture: A Quantative Approach*
