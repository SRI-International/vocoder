all:
	g++ -c Client.cpp Server.cpp VoIP.cpp
	g++ Client.o Server.o VoIP.o -o main -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network
clean:
	rm -r *.o main 2>/dev/null