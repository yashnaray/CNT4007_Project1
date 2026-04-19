CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -Iinclude 
LDFLAGS = -lws2_32 -pthread
TARGET = peerProcess
SRCS = main.cpp $(wildcard src/*.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f main.o src/*.o *.o $(TARGET)

.PHONY: all clean