# Makefile for Two Player Cooperative Console Game

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Source files
SOURCES = main.cpp Game.cpp Player.cpp Room.cpp Screen.cpp Point.cpp Utils.cpp GameObject.cpp Spring.cpp
HEADERS = Console.h Constants.h Game.h Layouts.h GameObject.h Player.h Point.h Room.h Screen.h Utils.h Spring.h

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Executable name
TARGET = game

# Platform detection
ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
    RM = del /Q
else
    RM = rm -f
endif

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

# Compile
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	$(RM) $(OBJECTS) $(TARGET)

# Run
run: $(TARGET)
	./$(TARGET)

# Phony targets
.PHONY: all clean run
