#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <iostream>

#define SOCKADDR_IN struct sockaddr_in 
#define SOCKADDR struct sockaddr 

using namespace std;

int clients[1024];
int connectedCount = 0;

template <std::size_t N>
int execvp(const char* file, const char* const (&argv)[N]){
    assert((N > 0) && (argv[N - 1] == nullptr));
    return execvp(file, const_cast<char* const*>(argv));
}

void handleClient(int socket){
    char buffer[33];
    sprintf(buffer, "%d", socket);
    const char* command = "./clientHandler";
    const char* const argument_list[] = {"./clientHandler", buffer, NULL};
    int i = fork();
    if(i == 0) execvp(command, argument_list);
}

int main(){
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    unsigned int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    saddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    int error = bind(s, (SOCKADDR*)&saddr, sizeof(saddr));
    if(error < 0){
        printf("Failed to bind to port number 8888\n");
        close(s);
        return 0;
    }
    listen(s, 10);
    while(1){
        int c = accept(s, (SOCKADDR*)&caddr, &clen);
        if (c > 0)
        {
            cout << "A client connected." << endl;
            clients[connectedCount] = c;
            connectedCount++;
            handleClient(c);
        }
    }
}