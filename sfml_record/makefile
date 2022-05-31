# Single-source SFML makefile
CXX      = g++
CXX_FILE = $(wildcard *.cpp)
TARGET   = $(patsubst %.cpp,%,$(CXX_FILE))
CXXFLAGS = -g -pedantic-errors -fmessage-length=0
LINKS    = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

all:
	$(CXX) $(CXXFLAGS) $(CXX_FILE) -o $(TARGET) $(LINKS)
clean:
	rm -f $(TARGET) $(TARGET).exe *.ogg *.wav