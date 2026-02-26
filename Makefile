CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -Iinclude
TARGET = program
SRCS = main.cpp $(wildcard src/*.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f main.o src/*.o $(TARGET)

.PHONY: all clean
