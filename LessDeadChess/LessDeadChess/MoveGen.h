#pragma once
#include "Defines.h"
#include "Board.h"
#include "FastStack.h"
#include <stdexcept>
#include <algorithm>


enum class MoveFlags : BYTE {
	SpecFlag0 = 0b1,
	SpecFlag1 = 0b10,
	CaptureFlag = 0b100,
	PromoFlag = 0b1000,

	Quiet = 0,
	DoublePawn = 1,
	KCastle = 2,
	QCastle = 3,
	Capture = 4,
	EPCapture = 5,
	KnightPromo = 8,
	BishopPromo = 9,
	RookPromo = 10,
	QueenPromo = 11,
	KnightPromoCapture = 12,
	BishopPromoCapture = 13,
	RookPromoCapture = 14,
	QueenPromoCapture = 15,
};

enum class MoveStorageFlags : BYTE {
	None = 0,
	CaptureFlag = 0b1,
	EnPassantFlag = 0b10,
	CRightsFlag = 0b100,
	HMClockFlag = 0b1000
};

enum class MoveGenerationStage : BYTE {
	PawnStage,
	RookLikeStage,
	BishopLikeStage,
	KnightStage,
	KingStage,
	FinalStage
};

enum class GeneratedNodeResult {
	None,
	Checkmate,
	Stalemate,
	Draw50Moves
};

class Move {
private:
	WORD mData;
public:
	Move(Square from, Square to, MoveFlags flags) {
		mData = (((BYTE)flags & 0xf) << 12) | ((from & 0x3f) << 6) | (to & 0x3f);
	}
	inline Square getTo() const { return (Square)(mData & 0x3f); }
	inline Square getFrom() const { return (Square)((mData >> 6) & 0x3f); }
	inline MoveFlags getFlags() const { return (MoveFlags)((mData >> 12) & 0x0f); }
	void setTo(Square to) {
		mData &= ~0x3f; 
		mData |= to & 0x3f; 
	}
	void setFrom(Square from) {
		mData &= ~0xfc0; 
		mData |= (from & 0x3f) << 6; 
	}
};

struct PlyGenerationData {
	BYTE moveCount = 0;
	MoveStorageFlags storageFlags = MoveStorageFlags::None;
	MoveGenerationStage generationStage = (MoveGenerationStage)0;
};

struct Position
{
public:
	Board board;
	Color turn;
	CRightsFlags cRights;
	EnPassant enPassant;
	BYTE hmClock;
	std::string toDebugAsciiView();
	// TODO: generate positon from FEN.
};

template<size_t size>
class MoveGen {
private:
	Position mPos;
	FastStack<PlyGenerationData, size> mHistoryStack;
	FastStack<Move, size * 64> mMoveStack;
	FastStack<Piece, std::min(size, (size_t)32)> mCaptureStack;
	FastStack<BYTE, std::min(size, (size_t)120)> mHMClockStack;
	FastStack<EnPassant, std::min(size, (size_t)16)> mEPStack;
	FastStack<CRightsFlags, std::min(size, (size_t)4)> mCRightsStack;

	void generatePawnMovesToBuffer();
	void generateRooklikeMovesToBuffer();
	void generateBishoplikeMovesToBuffer();
	void generateKnightMovesToBuffer();
	void generateKingMovesToBuffer();

	inline bool tryGenerateNextStageMoves(MoveGenerationStage& stage) {
		if (stage == MoveGenerationStage::FinalStage) return false;
		switch (stage)
		{
		case MoveGenerationStage::PawnStage:
			generatePawnMovesToBuffer();
			break;
		case MoveGenerationStage::RookLikeStage:
			generateRooklikeMovesToBuffer();
			break;
		case MoveGenerationStage::BishopLikeStage:
			generateBishoplikeMovesToBuffer();
			break;
		case MoveGenerationStage::KnightStage:
			generateKnightMovesToBuffer();
			break;
		case MoveGenerationStage::KingStage:
			generateKingMovesToBuffer();
			break;
		default:
			throw "invalid stage";
		}
		stage = (MoveGenerationStage)((BYTE)stage + 1);
		return true;
	}

	MoveStorageFlags makeMove(const Move move);
	void unmakeMove(const Move move, const MoveStorageFlags storageFlags);
public:
	inline const Position& getPosition() const { return mPos; }
	inline void resetRoot(Position root) {
		mPos = root;
		mHistoryStack.clear();
		mMoveStack.clear();
		mCaptureStack.clear();
		mHMClockStack.clear();
		mEPStack.clear();
		mCRightsStack.clear();
	}
	inline void abandonMoves() {
		PlyGenerationData generationData = mHistoryStack.pop();
		unmakeMove(mMoveStack[mMoveStack.length() - 1], generationData.storageFlags);
		mMoveStack.remove(generationData.moveCount);
	}
	GeneratedNodeResult generateMoves();
	bool tryApplyNextMoveAndFinishOtherwise();
};
