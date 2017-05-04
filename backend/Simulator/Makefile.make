
CC=$(CXX)
CFLAGS  = -g -Wall

all: simulator

# To create the executable file we need the object files
simulator: 
	$(CC) $(CFLAGS) -o simulator  simulator.o common.o message.o context.o processor.o protocolHandler.o MSIhandler.o directory.o cache.o

clean: 
	$(RM) simulator *.o *~


# "all" is name of the default target, running "make" without params would use it


# for C++, replace CC (c compiler) with CXX (c++ compiler) which is used as default linker
CC=$(CXX)

# tell which files should be used, .cpp -> .o make would do automatically
executable1: file1.o file2.o