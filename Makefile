server: clientHandler src/server.cpp
	g++ src/server.cpp -o server

clientHandler: src/clientHandler.cpp
	g++ src/clientHandler.cpp -o clientHandler