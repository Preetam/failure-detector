BUILD_SOURCES := main.cc

# Includes
INCLUDE_DIRS := ./include/
INCLUDE_DIRS += ./libs/
INCLUDE_DIRS_FLAGS := $(foreach d, $(INCLUDE_DIRS), -I$d)

FLAGS = -std=c++14 -Wall

LINK_FLAGS = -lpthread

BUILD_BINARY = ./failure-detector

all: build

clean:
	rm ./failure-detector

build:
	$(CXX) $(FLAGS) $(BUILD_SOURCES) $(INCLUDE_DIRS_FLAGS) $(LINK_FLAGS) -o $(BUILD_BINARY)

.PHONY: build
