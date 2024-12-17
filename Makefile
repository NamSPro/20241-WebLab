server: clientHandler server.cpp
	g++ server.cpp -o server

clientHandler: clientHandler.cpp
	g++ clientHandler.cpp -o clientHandler