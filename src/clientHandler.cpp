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
#include <errno.h>

#include <fstream>
#include "../lib/json.hpp"
#include "enums.h"
#include "../lib/chess/chess_main.h"

using namespace std;
using json = nlohmann::json;

int c; // client's socket number
const char* invalidCommand = "Invalid command.\n";
json accounts = {}, onlineAccounts = {}, ratings = {};
bool login = false;
char currentAccount[1024] = { 0 };
int opponentCount = 0;
char availableOpponents[1024][1024] = { 0 };

void inputCheck(int i){
    if(errno == ENOTCONN || i == 0){
        onlineAccounts.erase(currentAccount);
        saveJson(onlineAccounts, "db/online.json");
        close(c);
        exit(0);
    }
}

bool opponentInputCheck(int i){
    const char* opponentGone = "Opponent disconnected.";
    if(errno == ENOTCONN || i == 0){
        send(c, opponentGone, strlen(opponentGone), 0);
        return false;
    }
    return true;
}

void handleLogin(){
    const char* usernamePrompt = "Enter username: ";
    const char* passwordPrompt = "Enter password: ";
    char usernameBuffer[1024] = { 0 };
    char passwordBuffer[1024] = { 0 };
    const char* accountNotFound = "Account not found.\n\n\n";
    const char* loginFailure = "Wrong password.\n\n\n";
    const char* loginSuccess = "Login successful.\n\n\n";

    send(c, usernamePrompt, strlen(usernamePrompt), 0);
    recv(c, usernameBuffer, sizeof(usernameBuffer) - 1, 0);
    while(usernameBuffer[strlen(usernameBuffer) - 1] == '\r' || usernameBuffer[strlen(usernameBuffer) - 1] == '\n')
        usernameBuffer[strlen(usernameBuffer) - 1] = 0;
    send(c, passwordPrompt, strlen(passwordPrompt), 0);
    recv(c, passwordBuffer, sizeof(passwordBuffer) - 1, 0);
    while(passwordBuffer[strlen(passwordBuffer) - 1] == '\r' || passwordBuffer[strlen(passwordBuffer) - 1] == '\n')
        passwordBuffer[strlen(passwordBuffer) - 1] = 0;

    loadJson(accounts, "db/accounts.json");
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
        login = true;
        strcpy(currentAccount, usernameBuffer);
    }
}

void handleRegister(){
    const char* usernamePrompt = "Enter username: ";
    const char* passwordPrompt = "Enter password: ";
    const char* passwordPrompt2 = "Enter password again: ";
    char usernameBuffer[1024] = { 0 };
    char passwordBuffer[1024] = { 0 };
    char passwordBuffer2[1024] = { 0 };
    const char* invalidInput = "Invalid input.\n\n\n";
    const char* passwordMismatch = "Password doesnt match.\n\n\n";
    const char* accountExist = "Account already exists.\n\n\n";
    const char* registerSuccess = "Registration successful. Going back to login menu.\n\n\n";

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

    if(!strcmp(usernameBuffer, "") || !strcmp(passwordBuffer, "")){
        send(c, invalidInput, strlen(invalidInput), 0);
        return;
    }
    if(strcmp(passwordBuffer, passwordBuffer2) != 0){
        send(c, passwordMismatch, strlen(passwordMismatch), 0);
        return;
    }
    loadJson(accounts, "db/accounts.json");
    if(!accounts[usernameBuffer].json::is_null()){
        send(c, accountExist, strlen(accountExist), 0);
        return;
    }
    send(c, registerSuccess, strlen(registerSuccess), 0);
    accounts[usernameBuffer] = passwordBuffer;
    saveJson(accounts, "db/accounts.json");
    loadJson(ratings, "db/rating.json");
    ratings.push_back({{"name", usernameBuffer}, {"rating", 0}});
    saveJson(ratings, "db/rating.json");
    return;
}

void loginMenu(){
    const char* welcome = "Available functions:\n1. Login\n2. Register\n3. Quit\nYour choice: ";
    const char* goodbye = "Goodbye.";

    while(1){
        send(c, welcome, strlen(welcome), 0);
        char buffer[1024] = { 0 };
        int i = recv(c, buffer, sizeof(buffer) - 1, 0);
        inputCheck(i);
        while(buffer[strlen(buffer) - 1] == '\r' || buffer[strlen(buffer) - 1] == '\n') buffer[strlen(buffer) - 1] = 0;
        int cmd = atoi(buffer);
        switch (cmd){
            case 1:
                handleLogin();
                if(login) return;
                break;
            case 2:
                handleRegister();
                break;
            case 3:
                send(c, goodbye, strlen(goodbye), 0);
                close(c);
                return;
            default:
                send(c, invalidCommand, strlen(invalidCommand), 0);
                continue;
        }
    }
}

bool ratingCmp(json a, json b){
    if(a["rating"] == b["rating"]) return a < b;
    return a["rating"] > b["rating"];
}

void findOpponents(){
    json ratings;
    opponentCount = 0;
    loadJson(onlineAccounts, "db/online.json");
    loadJson(ratings, "db/rating.json");
    sort(ratings.begin(), ratings.end(), ratingCmp);
    saveJson(ratings, "db/rating.json");
    int currentRank = -1, tmp = -1;
    while(1){
        tmp++;
        if(tmp == ratings.size()) break;
        if(ratings[tmp]["name"] == currentAccount){
            if(currentRank != -1) continue;
            currentRank = tmp;
            tmp -= 10;
            if(tmp < 0) tmp = 0;
        }
        if(currentRank != -1 || abs(currentRank - tmp) <= 10){
            string opponentName = ratings[tmp]["name"];
            if(opponentName == currentAccount) continue;
            if(onlineAccounts[opponentName]["status"] != STATUS_ONLINE) continue;
            strcpy(availableOpponents[opponentCount], opponentName.c_str());
            opponentCount++;
        }
    }
    return;
}

void sendChallenge(char opponent[]){
    const char* screenClear = "\033[2J\033[1;1H";
    const char* waiting = "Waiting for opponent to respond...";
    const char *challenge = "User ", *challengePt2 = " has challenged you to a match! Accept? (y/n)";
    
    loadJson(onlineAccounts, "db/online.json");
    int opponentSocket = onlineAccounts[opponent]["socket"];
    onlineAccounts[currentAccount]["status"] = onlineAccounts[opponent]["status"] = STATUS_CHALLENGED;
    saveJson(onlineAccounts, "db/online.json");
    send(c, screenClear, strlen(screenClear), 0);
    send(c, waiting, strlen(waiting), 0);
    send(opponentSocket, screenClear, strlen(screenClear), 0);
    send(opponentSocket, challenge, strlen(challenge), 0);
    send(opponentSocket, currentAccount, strlen(currentAccount), 0);
    send(opponentSocket, challengePt2, strlen(challengePt2), 0);
    char buffer[1024] = { 0 };
    recv(opponentSocket, buffer, sizeof(buffer) - 1, 0);
    while(buffer[strlen(buffer) - 1] == '\r' || buffer[strlen(buffer) - 1] == '\n') buffer[strlen(buffer) - 1] = 0;
    if(buffer[0] == 'y'){
        onlineAccounts[currentAccount]["status"] = onlineAccounts[opponent]["status"] = STATUS_INGAME;
        saveJson(onlineAccounts, "db/online.json");
        send(c, screenClear, strlen(screenClear), 0);
        send(opponentSocket, screenClear, strlen(screenClear), 0);
        send(c, "Game started!\n", strlen("Game started!\n"), 0);
        send(opponentSocket, "Game started!\n", strlen("Game started!\n"), 0);
        chessMain(currentAccount, opponent);
    }
    else{
        onlineAccounts[currentAccount]["status"] = onlineAccounts[opponent]["status"] = STATUS_ONLINE;
        saveJson(onlineAccounts, "db/online.json");
        send(c, screenClear, strlen(screenClear), 0);
        send(c, "Challenge rejected.\n", strlen("Challenge rejected.\n"), 0);
        send(opponentSocket, screenClear, strlen(screenClear), 0);
        send(opponentSocket, "Challenge rejected.\n", strlen("Challenge rejected.\n"), 0);
    }
    return;
}

void mainMenu(){
    char welcome[1024] = "Welcome, \0", newLine[3] = ".\n";
    const char* welcomeFull = strcat(strcat(welcome, currentAccount), newLine);
    const char* menu = "1. Show online users\n2. Send a challenge\nYour choice: ";
    const char* availableOpponent = "Available opponents: \n";
    const char* selectOpponent = "Your choice: ";

    assert(login);

    loadJson(onlineAccounts, "db/online.json");
    onlineAccounts[currentAccount] = {{"status", STATUS_ONLINE}, {"socket", c}};
    saveJson(onlineAccounts, "db/online.json");

    send(c, welcomeFull, strlen(welcomeFull), 0);
    while(1){
        loadJson(onlineAccounts, "db/online.json");
        if(onlineAccounts[currentAccount]["status"] == STATUS_CHALLENGED || onlineAccounts[currentAccount]["status"] == STATUS_INGAME){
            sleep(1000);
            continue;
        }
        send(c, menu, strlen(menu), 0);
        char buffer[1024] = { 0 };
        int i = recv(c, buffer, sizeof(buffer) - 1, 0);
        inputCheck(i);
        while(buffer[strlen(buffer) - 1] == '\r' || buffer[strlen(buffer) - 1] == '\n') buffer[strlen(buffer) - 1] = 0;
        int cmd = atoi(buffer);
        if(onlineAccounts[currentAccount]["status"] == STATUS_CHALLENGED || onlineAccounts[currentAccount]["status"] == STATUS_INGAME){
            sleep(1000);
            continue;
        }
        switch(cmd){
            case 1:
                loadJson(onlineAccounts, "db/online.json");
                for(auto& i: onlineAccounts.items()){
                    if(i.value()["status"].is_null() || i.value()["status"] == STATUS_OFFLINE) continue;
                    string tmp = i.key() + "\n";
                    send(c, tmp.c_str(), strlen(tmp.c_str()), 0);
                }
                break;
            case 2:
                findOpponents();
                send(c, availableOpponent, strlen(availableOpponent), 0);
                for(int i = 0; i < opponentCount; i++){
                    char numberBuffer[12];
                    sprintf(numberBuffer, "%d. ", i + 1);
                    send(c, numberBuffer, strlen(numberBuffer), 0);
                    send(c, availableOpponents[i], strlen(availableOpponents[i]), 0);
                    send(c, newLine, strlen(newLine), 0);
                    send(c, selectOpponent, strlen(selectOpponent), 0);
                    i = recv(c, buffer, sizeof(buffer) - 1, 0);
                    inputCheck(i);
                    while(buffer[strlen(buffer) - 1] == '\r' || buffer[strlen(buffer) - 1] == '\n') buffer[strlen(buffer) - 1] = 0;
                    cmd = atoi(buffer);
                    cmd--;
                    if(cmd < 0 || cmd >= opponentCount){
                        send(c, invalidCommand, strlen(invalidCommand), 0);
                        continue;
                    }
                    sendChallenge(availableOpponents[cmd]);
                }
                break;
            default:
                send(c, invalidCommand, strlen(invalidCommand), 0);
                continue;
        }
    }
    return;
}

int main(int argc, char* argv[]){
    c = atoi(argv[1]);
    loginMenu();
    if(!login) return 0;
    mainMenu();
    return 0;
}