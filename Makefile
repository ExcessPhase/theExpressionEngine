ifndef BOOST_ROOT
$(error BOOST_ROOT is not set)
endif

all: theExpressionEngine.exe test.exe
CXXFLAGS=-march=native -ffast-math -I $(BOOST_ROOT)/include $(shell llvm-config-20 --cxxflags) -fexceptions -MMD -MP -std=c++20 -Wno-dangling-else -Wno-nan-infinity-disabled
#CXXFLAGS+=-O3 -DNDEBUG
CXXFLAGS+=-g -DDEBUG -O0 -fno-inline -fno-omit-frame-pointer
#CXXFLAGS+=-fsanitize=thread
CXX=clang++-20
#CXX=g++-13
LDFLAGS=$(shell llvm-config-20 --ldflags --system-libs --libs core irreader bitreader bitwriter support executionengine target)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

OBJECTS=theExpressionEngine.o
TEST_OBJECTS=test.o

DEPS = $(OBJECTS:.o=.d) $(TEST_OBJECTS:.o=.d)

clean:
	rm -f theExpressionEngine.exe $(OBJECTS) $(DEPS) $(TEST_OBJECTS)

theExpressionEngine.exe:$(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

test.exe:$(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJECTS)

-include $(DEPS)
