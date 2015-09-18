SRC      := ${shell find ./src -name *.cc}
TEST_SRC := ${shell find ./test -name *.cc}
OBJ      := ${SRC:.cc=.o}
TEST_OBJ := ${TEST_SRC:.cc=.o}
INCL     := ${shell find ./include}

CXXFLAGS = -std=c++14 -I./src -I./include -fPIC
LD_FLAGS = -L./build -lpthread

all: prepare build/failure-detector build/test

obj/%.o: src/%.cc
	$(CXX) -std=c++14 -fPIC -c -o $@ $<

obj/%.o: test/%.cc
	$(CXX) -std=c++14 -fPIC -c -o $@ $<

build/failure-detector: prepare $(OBJ) $(INCL)
	$(CXX) -fPIC -o $@ $(OBJ)

build/test: $(TEST_OBJ) $(INCL)
	$(CXX) -o $@ $(TEST_OBJ) $(LD_FLAGS)

prepare:
	mkdir -p build

clean:
	rm -rf ./build
	rm src/*.o
	rm test/*.o

test: build/test
	LD_LIBRARY_PATH=./build ./build/test -s -i

valgrind_test: build/test
	LD_LIBRARY_PATH=./build valgrind ./build/test

.PHONY: prepare clean test valgrind_test
