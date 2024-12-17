#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <cstring>

#define SOCKADDR_IN struct sockaddr_in 
#define SOCKADDR struct sockaddr 

using namespace std;

int main()
{
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    unsigned int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    saddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    int error = bind(s, (SOCKADDR*)&saddr, sizeof(saddr));
    if (error < 0)
    {
        printf("Failed to bind to port number 8888\n");
        close(s);
    }else
    {
        listen(s, 10);
        while (0 == 0)
        {
            int c = accept(s, (SOCKADDR*)&caddr, &clen);
            if (c > 0)
            {
                const char* welcome = "Hello World TCP Socket Programming!\n";
                int sent = send(c, welcome, strlen(welcome), 0);
                printf("Sent: %d (Bytes)\n", sent);    
                char buffer[1024] = { 0 };
                int received = recv(c, buffer, sizeof(buffer) - 1, 0);
                printf("Received: %d (Bytes)\n", received);
                printf("%s\n", buffer);
                close(c);
            }
        }
    }
}