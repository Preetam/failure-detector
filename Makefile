SRC      := ${shell find ./src -name *.cc}
TEST_SRC := ${shell find ./test -name *.cc}
OBJ      := ${SRC:.cc=.o}
TEST_OBJ := test/_test.o \
            test/encoding_test.o \
            test/message_test.o \
            src/message/message.o

INCL     := ${shell find ./include}
INCL     += ${shell find ./src -name *.hpp}

CXXFLAGS = -std=c++14 -I./src -I./include -fPIC
LD_FLAGS = -L./build -lglog -luv -lpthread

debug: CXXFLAGS += -DDEBUG -g
debug: build/failure-detector

all: prepare build/failure-detector build/test

src/%.o: src/%.cc src/%.hpp
	$(CXX) $(CXXFLAGS) -std=c++14 -fPIC -c -o $@ $<

test/%.o: test/%.cc test/%.hpp
	$(CXX) $(CXXFLAGS) -std=c++14 -fPIC -c -o $@ $<

build/failure-detector: prepare $(OBJ)
	$(CXX) -fPIC -o $@ $(OBJ) $(LD_FLAGS)

build/test: $(TEST_OBJ)
	$(CXX) -o $@ $(TEST_OBJ) $(LD_FLAGS)

prepare:
	mkdir -p build

clean:
	rm -rf ./build
	find . -name *.o | xargs rm

test: prepare build/test
	LD_LIBRARY_PATH=./build ./build/test

valgrind_test: build/test
	LD_LIBRARY_PATH=./build valgrind ./build/test

.PHONY: prepare clean test valgrind_test
