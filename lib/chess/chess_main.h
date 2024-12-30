#pragma once

#include "includes.h"
#include "user_interface.h"
#include "chess.h"
#include "../json.hpp"

using json = nlohmann::json;

void loadJson(json& target, string path);
void saveJson(json& target, string path);
bool isMoveValid(Chess::Position present, Chess::Position future, Chess::EnPassant* S_enPassant, Chess::Castling* S_castling, Chess::Promotion* S_promotion);
void makeTheMove(Chess::Position present, Chess::Position future, Chess::EnPassant* S_enPassant, Chess::Castling* S_castling, Chess::Promotion* S_promotion);
void newGame(void);
void undoMove(void);
void movePiece(void);
void saveGame(char player1[], char player2[]);
int chessMain(char player1[], char player2[]);