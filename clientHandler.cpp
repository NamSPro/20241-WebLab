#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <cstring>
#include <cstdlib>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]){
    int c = atoi(argv[1]);
    const char* welcome = "Hello World Client Handling!\n";
    while(1){
        char buffer[1024] = { 0 };
        recv(c, buffer, sizeof(buffer) - 1, 0);
        while(buffer[strlen(buffer) - 1] == '\r' || buffer[strlen(buffer) - 1] == '\n') buffer[strlen(buffer) - 1] = 0;
        send(c, buffer, strlen(buffer), 0);
    }
    return 0;
}