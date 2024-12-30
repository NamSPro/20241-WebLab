#pragma once
#include "chess.h"



// -------------------------------------------------------------------------------
// Macros will be of type const char* because theu are defined as string literals.
// -------------------------------------------------------------------------------
#define WHITE_SQUARE_WINDOWS "\xDB" 
#define BLACK_SQUARE_WINDOWS "\xFF"

#define WHITE_SQUARE_LINUX " "
#define BLACK_SQUARE_LINUX "█"

#define WHITE_SQUARE WHITE_SQUARE_WINDOWS
#define BLACK_SQUARE BLACK_SQUARE_WINDOWS


#define EMPTY_SQUARE 0x20

void createNextMessage( string msg );
void appendToNextMessage( string msg );
void clearScreen( int whiteSocket, int blackSocket );
void printLogo( void );
void printLogo( void );
void printMenu( void );
void printMessage( void );
string printLine(int iLine, const char* pchColor1, const char* pchColor2, Game& game);
void printSituation( Game& game, int whiteSocket, int blackSocket );
void printBoard( Game& game, int whiteSocket, int blackSocket );