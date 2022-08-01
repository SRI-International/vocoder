CXX      = g++
CXX_FILE = main.cpp
TARGET   = main
#CFLAGS   = -g -DMINIUPNP_STATICLIB -DNDEBUG -s -O2
CXXFLAGS = -g -Wall -fmessage-length=0 -s -O2
LINKS    = -lportaudio -lopus -lcodec2 -llog4cpp

all:
	$(CXX) $(CXXFLAGS) $(CXX_FILE) -o $(TARGET) $(LINKS)
clean:
	rm -f $(TARGET) $(TARGET).exe