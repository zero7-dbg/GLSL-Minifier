
CXX = g++

CXXFLAGS = -O3 -std=c++23 -Wall -Wextra

TARGET = glsl_minifier

SRCS = main.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)


%.o: %.cpp GLSLminifier.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe min_*.glsl

.PHONY: all run clean