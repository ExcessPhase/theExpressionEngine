all: theExpressionEngine.exe test.exe
#CXXFLAGS=-O3 -DNDEBUG -march=native -ffast-math -I $(BOOST_ROOT)/include $(shell llvm-config --cxxflags) -std=c++17 -fexceptions -MMD -MP
CXXFLAGS=-g -DNDEBUG -march=native -ffast-math -I $(BOOST_ROOT)/include $(shell llvm-config --cxxflags) -std=c++17 -fexceptions -MMD -MP -fsanitize=address
CXX=clang++
LDFLAGS=$(shell llvm-config --ldflags --system-libs --libs core irreader bitreader bitwriter support executionengine target)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

OBJECTS=expression.o bison.o factory.o theExpressionEngine.o
TEST_OBJECTS=expression.o bison.o factory.o test.o

DEPS = $(OBJECTS:.o=.d) $(TEST_OBJECTS:.o=.d)

clean:
	rm -f theExpressionEngine.exe $(OBJECTS) $(DEPS) bison.cpp bison.h flex.cpp $(TEST_OBJECTS)

theExpressionEngine.exe:$(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

bison.cpp bison.h: bison.y
	bison --header=bison.h --output=bison.cpp bison.y

flex.cpp: flex.l
	flex --reentrant --outfile=flex.cpp flex.l

factory.o:flex.cpp

test.exe:$(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJECTS)
-include $(DEPS)
