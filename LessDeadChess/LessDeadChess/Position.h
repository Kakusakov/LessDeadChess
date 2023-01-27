#pragma once
#include "Board.h"

const std::string classicStartPosFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

struct Position
{
public:
	Board board;
	Color turn;
	CRightsFlags cRights;
	EnPassant enPassant;
	BYTE hmClock;

	std::string toDebugAsciiView() const;
	Position(std::string fen);
};