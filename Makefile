CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
TARGET = compiler

all: $(TARGET)

$(TARGET): compiler.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) compiler.cpp

clean:
	rm -f $(TARGET) *.exe

.PHONY: all clean
