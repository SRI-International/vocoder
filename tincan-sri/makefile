CXX      = g++
TARGET   = tincanphone
# CFLAGS   = -g -DMINIUPNP_STATICLIB -DNDEBUG -s -O2
CXXFLAGS = -g -Wall -fmessage-length=0 -DNDEBUG -s -O2
LINKS    = `pkg-config --cflags --libs gtk+-3.0 opus portaudio-2.0` -lrt -lm -lasound -ljack -pthread

all:
	$(CXX) -o $(TARGET) `ls src/*.cpp` `ls src/Gtk/*.cpp` -Isrc/ $(LINKS) $(CXXFLAGS)

clean:
	rm -f $(TARGET) $(TARGET).exe 