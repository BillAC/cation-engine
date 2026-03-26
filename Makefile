CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
INCLUDES = -Iinclude
SOURCES = src/LigandCSVLoader.cpp src/CationEngine.cpp src/Solver.cpp src/LigandData.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLES = cation_engine_main cation_engine_test

all: $(EXECUTABLES)

cation_engine_main: main.cpp $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(OBJECTS)

cation_engine_test: test_ligand_loading.cpp $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(EXECUTABLES)

.PHONY: all clean