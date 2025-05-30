ifndef BOOST_ROOT
$(error BOOST_ROOT is not set)
endif

all: theExpressionEngine.exe test.exe
CXXFLAGS=-O3 -DNDEBUG -march=native -ffast-math -I $(BOOST_ROOT)/include $(shell llvm-config --cxxflags) -fexceptions -MMD -MP -std=c++17 -Wno-dangling-else
#CXXFLAGS=-g -DDEBUG -march=native -ffast-math -I $(BOOST_ROOT)/include $(shell llvm-config --cxxflags) -fexceptions -MMD -MP -std=c++17 -fsanitize=memory -fno-inline -O0 -fno-omit-frame-pointer -Wno-dangling-else
CXX=clang++
#CXX=g++-13
LDFLAGS=$(shell llvm-config --ldflags --system-libs --libs core irreader bitreader bitwriter support executionengine target)

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
