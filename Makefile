#!/usr/bin/env make

SHELL := /usr/bin/bash

TARGET = mediafind

CXX = g++
CXXSTD = c++26
CXXFLAGS =  -Wall -Wextra -std=$(CXXSTD) -march=native -O3 -pthread
CXXDEBUGFLAGS =  -O0 -g -Wall -Wextra -std=gnu++26 -D_GLIBCXX_DEBUG -pthread  -fsanitize=thread

.PHONY: clean test


mediafind:	mediafind.hpp mediafind.cpp
	$(CXX) $(CXXFLAGS) -o mediafind mediafind.cpp


debug:	mediafind.hpp mediafind.cpp
	$(CXX) $(CXXDEBUGFLAGS) -o mediafind mediafind.cpp


test:	mediafind
	./mediafind -d -a "progressive rock"
	./mediafind -d -v "street"

json:	mediafind
	./mediafind -m > mediafind.json
	cat mediafind.json |jq 

clean:
	rm -f *~ *.json $(TARGET)

