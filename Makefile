CXX=g++
CXXFLAGS=--std=c++11 -O3 -march=native

all: main.out

main.out: main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@
