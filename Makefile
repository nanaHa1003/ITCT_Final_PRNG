CXX=g++
CXXFLAGS=-O3 -march=native

all: main.out

main.out: main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@
