
PROJECT = simulator
CXX=g++
CXXFLAGS= -g -std=c++11 -O3
OBJECTS= main.o simulator.o context.o processor.o MSIHandler.o MESIHandler.o directory.o cache.o
INCLUDES= -I../../common/taskLib/
LIBS= -L../../common/taskLib/ -lTask -lz -L.
EXTERNALS=

all: taskLib $(PROJECT)

taskLib:
	$(MAKE) -C ../../common/taskLib

%.o : %.cpp
	$(CXX) $< $(CXXFLAGS) $(INCLUDES) -c -o $@
$(PROJECT): $(OBJECTS)
	$(CXX) -o $(PROJECT) $(OBJECTS) $(CXXFLAGS) $(LIBS)

clean:
	rm $(OBJECTS) $(PROJECT)
	rm *.txt
