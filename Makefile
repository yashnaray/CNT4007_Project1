CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -Iinclude
TARGET = peerProcess
SRCS = main.cpp $(wildcard src/*.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Add '-o $@' to the end of this line so it outputs to src/
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f main.o src/*.o *.o $(TARGET)

.PHONY: all clean