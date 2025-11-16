CXX = g++
CXXFLAGS = -std=c++11 -Wall
TARGET = compiler
SRC = parser.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
