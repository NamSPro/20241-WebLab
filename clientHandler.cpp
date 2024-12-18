#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include <fstream>
#include "lib/json.hpp"

using namespace std;
using json = nlohmann::json;

int c; // client's socket number
const char* invalidCommand = "Invalid command.\n";
json accounts = {};

void loadAccounts(){
    ifstream f("accounts.json");
    accounts = json::parse(f);
    f.close();
}

void saveAccounts(){
    ofstream f("accounts.json");
    f << accounts.dump(4);
    f.close();
}

void handleLogin(){
    const char* usernamePrompt = "Enter username: ";
    const char* passwordPrompt = "Enter password: ";
    char usernameBuffer[1024] = { 0 };
    char passwordBuffer[1024] = { 0 };
    const char* accountNotFound = "Account not found.\n\n\n";
    const char* loginFailure = "Wrong password.\n\n\n";
    const char* loginSuccess = "ok\n\n\n";

    send(c, usernamePrompt, strlen(usernamePrompt), 0);
    recv(c, usernameBuffer, sizeof(usernameBuffer) - 1, 0);
    while(usernameBuffer[strlen(usernameBuffer) - 1] == '\r' || usernameBuffer[strlen(usernameBuffer) - 1] == '\n')
        usernameBuffer[strlen(usernameBuffer) - 1] = 0;
    send(c, passwordPrompt, strlen(passwordPrompt), 0);
    recv(c, passwordBuffer, sizeof(passwordBuffer) - 1, 0);
    while(passwordBuffer[strlen(passwordBuffer) - 1] == '\r' || passwordBuffer[strlen(passwordBuffer) - 1] == '\n')
        passwordBuffer[strlen(passwordBuffer) - 1] = 0;

    loadAccounts();
    if(accounts[usernameBuffer].json::is_null()){
        send(c, accountNotFound, strlen(accountNotFound), 0);
        return;
    }
    string password = accounts[usernameBuffer];
    if(strcmp(password.c_str(), passwordBuffer) != 0){
        send(c, loginFailure, strlen(loginFailure), 0);
    }
    else{
        send(c, loginSuccess, strlen(loginSuccess), 0);
    }
}

void handleRegister(){
    const char* usernamePrompt = "Enter username: ";
    const char* passwordPrompt = "Enter password: ";
    const char* passwordPrompt2 = "Enter password again: ";
    char usernameBuffer[1024] = { 0 };
    char passwordBuffer[1024] = { 0 };
    char passwordBuffer2[1024] = { 0 };
    const char* passwordMismatch = "Password doesnt match.\n\n\n";
    const char* accountExist = "Account already exists.\n\n\n";
    const char* registerSuccess = "ok\n\n\n";

    send(c, usernamePrompt, strlen(usernamePrompt), 0);
    recv(c, usernameBuffer, sizeof(usernameBuffer) - 1, 0);
    while(usernameBuffer[strlen(usernameBuffer) - 1] == '\r' || usernameBuffer[strlen(usernameBuffer) - 1] == '\n')
        usernameBuffer[strlen(usernameBuffer) - 1] = 0;
    send(c, passwordPrompt, strlen(passwordPrompt), 0);
    recv(c, passwordBuffer, sizeof(passwordBuffer) - 1, 0);
    while(passwordBuffer[strlen(passwordBuffer) - 1] == '\r' || passwordBuffer[strlen(passwordBuffer) - 1] == '\n')
        passwordBuffer[strlen(passwordBuffer) - 1] = 0;
    send(c, passwordPrompt2, strlen(passwordPrompt2), 0);
    recv(c, passwordBuffer2, sizeof(passwordBuffer2) - 1, 0);
    while(passwordBuffer2[strlen(passwordBuffer2) - 1] == '\r' || passwordBuffer2[strlen(passwordBuffer2) - 1] == '\n')
        passwordBuffer2[strlen(passwordBuffer2) - 1] = 0;

    if(strcmp(passwordBuffer, passwordBuffer2) != 0){
        send(c, passwordMismatch, strlen(passwordMismatch), 0);
        return;
    }
    loadAccounts();
    if(!accounts[usernameBuffer].json::is_null()){
        send(c, accountExist, strlen(accountExist), 0);
        return;
    }
    send(c, registerSuccess, strlen(registerSuccess), 0);
    accounts[usernameBuffer] = passwordBuffer;
    saveAccounts();
    return;
}

int main(int argc, char* argv[]){
    c = atoi(argv[1]);
    const char* welcome = "Available functions:\n1. Login\n2. Register\n3. Quit\nYour choice: ";
    const char* goodbye = "Goodbye.";

    while(1){
        send(c, welcome, strlen(welcome), 0);
        char buffer[1024] = { 0 };
        recv(c, buffer, sizeof(buffer) - 1, 0);
        while(buffer[strlen(buffer) - 1] == '\r' || buffer[strlen(buffer) - 1] == '\n') buffer[strlen(buffer) - 1] = 0;
        int cmd = atoi(buffer);
        switch (cmd){
            case 1:
                handleLogin();
                break;
            case 2:
                handleRegister();
                break;
            case 3:
                send(c, goodbye, strlen(goodbye), 0);
                close(c);
                return 0;
            default:
                send(c, invalidCommand, strlen(invalidCommand), 0);
                continue;
        }
    }
    return 0;
}