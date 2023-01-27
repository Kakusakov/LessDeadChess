#pragma once
#include "MoveGen.h"

template<size_t max_depth>
class Search {
private:
	MoveGen<max_depth + 1> mMoveGen = { Position(classicStartPosFen) };
	int checkmateWeight = -200;
	int stalemateWeight = 0;
	int draw50movesWeight = 0;

	int NegaMax(int depth, int alpha, int beta);
	int Evaluate();
public:
	Position FindBestMove(Position root) {
		mMoveGen.resetRoot(root);
		if (mMoveGen.generateMoves() != GeneratedNodeResult::None) {
			throw std::logic_error("The root for the search is a terminal position");
		}
		Position bestMove = mMoveGen.getPosition();
		int bestScore = -NegaMax(max_depth - 1, checkmateWeight, -checkmateWeight);
		/*std::cout << mMoveGen.getPosition().toDebugAsciiView() 
			+ " score=" + std::to_string(bestScore) + "\n";*/
		while (mMoveGen.tryApplyNextMoveOrFinish()) {
			int score = -NegaMax(max_depth - 1, checkmateWeight, bestScore);
			/*std::cout << mMoveGen.getPosition().toDebugAsciiView()
				+ " score=" + std::to_string(score) + "\n";*/
			if (score > bestScore) {
				bestScore = score;
				bestMove = mMoveGen.getPosition();
			}
		}
		return bestMove;
	}
};

template<size_t max_depth>
int Search<max_depth>::NegaMax(int depth, int alpha, int beta) {
	switch (mMoveGen.generateMoves())
	{
	case GeneratedNodeResult::None: break;
	case GeneratedNodeResult::Checkmate: return checkmateWeight;
	case GeneratedNodeResult::Stalemate: return stalemateWeight;
	case GeneratedNodeResult::Draw50Moves: return draw50movesWeight;
	default: throw std::logic_error("Found unknow genereated node result");
	}
	if (depth <= 0) {
		int eval = Evaluate();
		mMoveGen.abandonMoves();
		return eval;
	}

	do {
		alpha = std::max(alpha, -NegaMax(depth - 1, -beta, -alpha));
		if (alpha >= beta) {
			mMoveGen.abandonMoves();
			return beta;
		}
	} while (mMoveGen.tryApplyNextMoveOrFinish());
	return alpha;
}

template<size_t max_depth>
int Search<max_depth>::Evaluate() {
	const Position& pos = mMoveGen.getPosition();
	const Color player = pos.turn;
	const Color opponent = (Color)((pos.turn + 1) & 1);
	return (popCount(pos.board.getColoredPieces(player, Piece::Pawn)) 
			- popCount(pos.board.getColoredPieces(opponent, Piece::Pawn))) * 1
		+ (popCount(pos.board.getColoredPieces(player, Piece::Bishop))
			- popCount(pos.board.getColoredPieces(opponent, Piece::Bishop))) * 3
		+ (popCount(pos.board.getColoredPieces(player, Piece::Knight))
			- popCount(pos.board.getColoredPieces(opponent, Piece::Knight))) * 3
		+ (popCount(pos.board.getColoredPieces(player, Piece::Rook))
			- popCount(pos.board.getColoredPieces(opponent, Piece::Rook))) * 5
		+ (popCount(pos.board.getColoredPieces(player, Piece::Queen))
			- popCount(pos.board.getColoredPieces(opponent, Piece::Queen))) * 9;
}

