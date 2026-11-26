CXX = g++
CXXFLAGS = -std=c++20 -O2

all: code

code: main.cpp game.h
	$(CXX) $(CXXFLAGS) -o code main.cpp

clean:
	rm -f code

.PHONY: all clean
