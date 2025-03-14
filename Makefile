all: theExpressionEngine.exe
CXXFLAGS=-O3 -DNDEBUG -march=native -ffast-math -I $(BOOST_ROOT)/include $(shell llvm-config --cxxflags) -std=c++17 -fexceptions -MMD -MP
CXX=clang++
LDFLAGS=$(shell llvm-config --ldflags --system-libs --libs core irreader bitreader bitwriter support executionengine target)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

OBJECTS=expression.o factory.o theExpressionEngine.o

DEPS = $(OBJECTS:.o=.d)

clean:
	rm -f theExpressionEngine.exe $(OBJECTS) $(DEPS)

theExpressionEngine.exe:$(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

-include $(DEPS)
