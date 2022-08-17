CXX      = g++
TARGET   = tincanphone
CFLAGS   = -g -DMINIUPNP_STATICLIB -DNDEBUG -s -O2
CXXFLAGS = -g -Wall -fmessage-length=0 -DMINIUPNP_STATICLIB -DNDEBUG -s -O2
LINKS    = `pkg-config --cflags --libs gtk+-3.0 opus portaudio-2.0` -lrt -lm -lasound -ljack -pthread

all:
	mkdir -p obj
	cd obj; gcc -c `ls ../src/miniupnpc/*.c` $(CFLAGS)
	ar rcs miniupnpc.a `ls obj/*.o`

	$(CXX) -o $(TARGET) `ls src/*.cpp` `ls src/Gtk/*.cpp` -Isrc/ miniupnpc.a $(LINKS) $(CXXFLAGS)

	rm -r obj/
	rm miniupnpc.a
clean:
	rm -f $(TARGET) $(TARGET).exe 