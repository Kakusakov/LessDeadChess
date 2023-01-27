#include "Search.h"
#include "Perft.h"
#include <iostream>
#include <chrono>

/*
CODE CLEANUP TASKS :
	MAIN TASKS:
	1. Finish TODOs.
	2. Assert-check all code contracts on DEBUG.
	3. Refactor for readability.

	OPTIONAL TASKS:
	1. Inline where sensible.
	2. Change enums to enum classes.
*/ 

#if defined LOG_TO_FILE 
#include <cstdio>
#include <fstream>
#include <ctime>

FILE* logger;

void setupFileLogging() {
	std::time_t t = std::time(0);
	std::tm now;
	localtime_s(&now, &t);
	std::string fileName = "EngineLog "
		+ std::to_string(now.tm_year + 1900) + '-'
		+ std::to_string(now.tm_mon + 1) + '-'
		+ std::to_string(now.tm_mday) + ' '
		+ std::to_string(now.tm_hour + 1) + '-'
		+ std::to_string(now.tm_min + 1) + '-'
		+ std::to_string(now.tm_sec + 1) + ".txt";
	std::ofstream file(fileName.c_str());
	if (file.good()) {
		file.close();
		freopen_s(&logger, fileName.c_str(), "w", stdout);
	}
	else {
		std::cerr << file.rdstate();
		throw std::logic_error("creating a logger failed");
	}
}
#endif

void initalizeEngine() {
#if defined DEBUG
	std::cout << "ENGINE >> started initializing engine" << std::endl;
#endif
	initalizeBoardClass();
#if defined DEBUG
	std::cout << "ENGINE >> finished initializing engine" << std::endl;
#endif
}

int main() {
#if defined LOG_TO_FILE 
	setupFileLogging();
#endif
	initalizeEngine();
	/*std::cout << "size of Board = " << sizeof(Board) << std::endl;
	std::cout << "size of Position = " << sizeof(Position) << std::endl;
	std::cout << "size of MoveFlags = " << sizeof(MoveFlags) << std::endl;
	std::cout << "size of Move = " << sizeof(Move) << std::endl;
	std::cout << "size of MoveGen<1> = " << sizeof(MoveGen<1>) << std::endl;
	std::cout << "size of MoveGen<2> = " << sizeof(MoveGen<2>) << std::endl;
	std::cout << "size of MoveGen<3> = " << sizeof(MoveGen<3>) << std::endl;
	std::cout << "size of MoveGen<5> = " << sizeof(MoveGen<5>) << std::endl;
	std::cout << "size of MoveGen<10> = " << sizeof(MoveGen<10>) << std::endl;
	std::cout << "size of MoveGen<20> = " << sizeof(MoveGen<20>) << std::endl;
	std::cout << "size of MoveGen<50> = " << sizeof(MoveGen<50>) << std::endl;*/

	/*Board board = {};
	board.createColoredPieces(Color::White, Piece::Pawn, C64(1) << 8);
	board.createColoredPieces(Color::White, Piece::Pawn, C64(1) << 10);
	board.createColoredPieces(Color::Black, Piece::Pawn, C64(1) << 40);
	std::cout << "popcount=" + std::to_string(popCount(board.getBB(Board::PieceBB::White))) + " whites: ";
	auto whites = serializeBB(board.getBB(Board::PieceBB::White));
	for (const auto& sq : whites) {
		std::cout << std::to_string(sq) + " ";
	}
	std::cout << "\n" + BBToString(board.getBB(Board::PieceBB::White)) + "\n";
	std::cout << "popcount=" + std::to_string(popCount(board.getBB(Board::PieceBB::Pawn))) + " pawns: ";
	auto pawns = serializeBB(board.getBB(Board::PieceBB::Pawn));
	for (const auto& sq : pawns) {
		std::cout << std::to_string(sq) + " ";
	}
	std::cout << "\n" + BBToString(board.getBB(Board::PieceBB::Pawn)) + "\n";*/

	/*Position startPos = Position(classic_start_pos_fen);
	MoveGen<1> moveGen = MoveGen<1>(startPos);
	std::cout << "\n" + moveGen.getPosition().toDebugAsciiView();
	auto res = moveGen.generateMoves();
	std::cout << "the initial node is evaluated as " + std::to_string((int)res);
	std::cout << " \nnext states:\n\n";
	int count = 0;
	do {
		std::cout << "\n" + moveGen.getPosition().toDebugAsciiView() + "\n";
		++count;
	} while (moveGen.tryApplyNextMoveAndFinishOtherwise());
	std::cout << "got " + std::to_string(count) + " total moves.\n";*/

	Position startPos = Position("r1bq1bkr/ppp3pp/2n5/3np3/2B5/5Q2/PPPP1PPP/RNB1K2R w KQ - 2 8");
	std::cout << startPos.toDebugAsciiView() + "\n\n";
	auto begin = std::chrono::high_resolution_clock::now();
	Search<5> search = {};
	std::cout << "best move=" + search.FindBestMove(startPos).toDebugAsciiView();
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
	std::cout << " found in " << elapsed.count() * 1e-9 << " sec \n\n";

	//const std::string testCastlingFen = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1";
	//handControllPerft(Position(testCastlingFen));

	return 0;
}
