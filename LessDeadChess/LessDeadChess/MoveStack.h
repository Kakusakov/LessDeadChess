//#pragma once
//#include "Board.h"
//#include "Position.h"
//#include "FastStack.h"
//#include <algorithm>
//
//
//enum class MoveFlags : BYTE {
//	SpecFlag0 = 0b1,
//	SpecFlag1 = 0b10,
//	CaptureFlag = 0b100,
//	PromoFlag = 0b1000,
//
//	Quiet = 0,
//	DoublePawn = 1,
//	KCastle = 2,
//	QCastle = 3,
//	Capture = 4,
//	EPCapture = 5,
//	KnightPromo = 8,
//	BishopPromo = 9,
//	RookPromo = 10,
//	QueenPromo = 11,
//	KnightPromoCapture = 12,
//	BishopPromoCapture = 13,
//	RookPromoCapture = 14,
//	QueenPromoCapture = 15,
//};
//inline MoveFlags operator&(MoveFlags a, MoveFlags b) {
//	return static_cast<MoveFlags>(static_cast<BYTE>(a) & static_cast<BYTE>(b));
//}
//inline MoveFlags operator|(MoveFlags a, MoveFlags b) {
//	return static_cast<MoveFlags>(static_cast<BYTE>(a) | static_cast<BYTE>(b));
//}
//inline MoveFlags operator^(MoveFlags a, MoveFlags b) {
//	return static_cast<MoveFlags>(static_cast<BYTE>(a) ^ static_cast<BYTE>(b));
//}
//
//enum class StorageFlags : BYTE {
//	None = 0,
//	CaptureFlag = 0b1,
//	EnPassantFlag = 0b10,
//	CRightsFlag = 0b100,
//	HMClockFlag = 0b1000
//};
//inline StorageFlags operator&(StorageFlags a, StorageFlags b) {
//	return static_cast<StorageFlags>(static_cast<BYTE>(a) & static_cast<BYTE>(b));
//}
//inline StorageFlags operator|(StorageFlags a, StorageFlags b) {
//	return static_cast<StorageFlags>(static_cast<BYTE>(a) | static_cast<BYTE>(b));
//}
//inline StorageFlags operator^(StorageFlags a, StorageFlags b) {
//	return static_cast<StorageFlags>(static_cast<BYTE>(a) ^ static_cast<BYTE>(b));
//}
//
//enum class MoveGenerationStage : BYTE {
//	PawnStage,
//	RookLikeStage,
//	BishopLikeStage,
//	KnightStage,
//	KingStage,
//	FinalStage
//};
//
//enum class GeneratedNodeResult {
//	None,
//	Checkmate,
//	Stalemate,
//	Draw50Moves
//};
//
//class Move {
//private:
//	WORD mData = 0;
//public:
//	Move() { mData = 0; }  // TODO: check if I can remove this.
//	Move(Square from, Square to, MoveFlags flags) {
//		mData = (((BYTE)flags & 0xf) << 12) | ((from & 0x3f) << 6) | (to & 0x3f);
//	}
//	inline Square getTo() const { return (Square)(mData & 0x3f); }
//	inline Square getFrom() const { return (Square)((mData >> 6) & 0x3f); }
//	inline MoveFlags getFlags() const { return (MoveFlags)((mData >> 12) & 0x0f); }
//	inline WORD getRawData() const { return mData; }
//};
//
//struct PlyHistory {
//	BYTE moveCount = 0;
//	StorageFlags storageFlags = StorageFlags::None;
//	MoveGenerationStage generationStage = (MoveGenerationStage)0;
//};
//
//
////template<size_t size>
////class MoveGen {
////private:
////	Position mPos;
////	FastStack<PlyGenerationData, size> mHistoryStack = {};
////	FastStack<Move, size * 64> mMoveStack = {};
////	FastStack<Piece, std::min(size, (size_t)32)> mCaptureStack = {};
////	FastStack<BYTE, std::min(size, (size_t)120)> mHMClockStack = {};
////	FastStack<EnPassant, std::min(size, (size_t)16)> mEPStack = {};
////	FastStack<CRightsFlags, std::min(size, (size_t)4)> mCRightsStack = {};
////
////	void generatePawnMovesToBuffer();
////	void generateRooklikeMovesToBuffer();
////	void generateBishoplikeMovesToBuffer();
////	void generateKnightMovesToBuffer();
////	void generateKingMovesToBuffer();
////
////	inline bool tryGenerateNextStageMoves(MoveGenerationStage& stage) {
////		if (stage == MoveGenerationStage::FinalStage) return false;
////		switch (stage)
////		{
////		case MoveGenerationStage::PawnStage:
////			generatePawnMovesToBuffer();
////			break;
////		case MoveGenerationStage::RookLikeStage:
////			generateRooklikeMovesToBuffer();
////			break;
////		case MoveGenerationStage::BishopLikeStage:
////			generateBishoplikeMovesToBuffer();
////			break;
////		case MoveGenerationStage::KnightStage:
////			generateKnightMovesToBuffer();
////			break;
////		case MoveGenerationStage::KingStage:
////			generateKingMovesToBuffer();
////			break;
////		default:
////			throw "invalid stage";
////		}
////		stage = (MoveGenerationStage)((BYTE)stage + 1);
////		return true;
////	}
////
////	bool TryMakeValidMoveOrFinish();
////
////	MoveStorageFlags makeMove(const Move move);
////	void unmakeMove(const Move move, const MoveStorageFlags storageFlags);
////public:
////	inline MoveGen(const Position pos) : mPos(pos) { }
////	inline const Position& getPosition() const { return mPos; }
////	inline void resetRoot(Position root) {
////		mPos = root;
////		mHistoryStack.clear();
////		mMoveStack.clear();
////		mCaptureStack.clear();
////		mHMClockStack.clear();
////		mEPStack.clear();
////		mCRightsStack.clear();
////	}
////	inline void abandonMoves() {
////		PlyGenerationData generationData = mHistoryStack.pop();
////		unmakeMove(mMoveStack[mMoveStack.length() - 1], generationData.storageFlags);
////		mMoveStack.remove(generationData.moveCount);
////	}
////	GeneratedNodeResult generateMoves();
////	bool tryApplyNextMoveOrFinish();
////	std::vector<Position> collectMovesAndFinish();
////};
//
//struct MoveStackDoubleIrrevStore : public std::exception {
//	const char* what() const throw ();
//};
//
//template<size_t maxPly>
//class MoveStack
//{
//private:
//	static constexpr size_t historyStackCap = maxPly;
//	static constexpr size_t moveStackCap = maxPly * 64;
//	static constexpr size_t captureStackCap = std::min(maxPly, (size_t)32);
//	static constexpr size_t HMClockStackCap = std::min(maxPly, (size_t)120);
//	static constexpr size_t EPStackCap = std::min(maxPly, (size_t)16);
//	static constexpr size_t CRightsStackCap = std::min(maxPly, (size_t)4);
//
//	FastStack<PlyHistory, historyStackCap> mHistoryStack = {};
//	FastStack<Move, moveStackCap> mMoveStack = {};
//	FastStack<Piece, captureStackCap> mCaptureStack = {};
//	FastStack<BYTE, HMClockStackCap> mHMClockStack = {};
//	FastStack<EnPassant, EPStackCap> mEPStack = {};
//	FastStack<CRightsFlags, CRightsStackCap> mCRightsStack = {};
//public:
//	inline const FastStack<PlyHistory, historyStackCap>& getHistoryStack() const { return mHistoryStack; }
//	inline const FastStack<Move, moveStackCap>& getMoveStack() const { return mMoveStack; }
//	inline const FastStack<Piece, captureStackCap>& getCaptureStack() const { return mCaptureStack; }
//	inline const FastStack<BYTE, HMClockStackCap>& getHMClockStack() const { return mHMClockStack; }
//	inline const FastStack<EnPassant, EPStackCap>& getEPStack() const { return mEPStack; }
//	inline const FastStack<CRightsFlags, CRightsStackCap>& getCRightsStack() const { return mCRightsStack; }
//
//	inline void PushMove(const Move& move) {
//		PlyHistory& ply = mHistoryStack.top();  // TODO: try awoid this.
//		ply.moveCount++;
//		mMoveStack.push(move);
//	}
//	inline void StoreCapture(const Piece& capture) {
//		PlyHistory& ply = mHistoryStack.top();  // TODO: try awoid this.
//		if (ply.storageFlags & StorageFlags::CaptureFlag != StorageFlags::None) throw MoveStackDoubleIrrevStore();
//		ply.storageFlags = ply.storageFlags | StorageFlags::CaptureFlag;
//		mCaptureStack.push(capture);
//	}
//	inline void StoreHMClock(const BYTE& hmClock) {
//		PlyHistory& ply = mHistoryStack.top();  // TODO: try awoid this.
//		if (ply.storageFlags & StorageFlags::HMClockFlag != StorageFlags::None) throw MoveStackDoubleIrrevStore();
//		ply.storageFlags = ply.storageFlags | StorageFlags::HMClockFlag;
//		mHMClockStack.push(hmClock);
//	}
//	inline void StoreEnPassant(const EnPassant& enPassant) {
//		PlyHistory& ply = mHistoryStack.top();  // TODO: try awoid this.
//		if (ply.storageFlags & StorageFlags::EnPassantFlag != StorageFlags::None) throw MoveStackDoubleIrrevStore();
//		ply.storageFlags = ply.storageFlags | StorageFlags::EnPassantFlag;
//		mEPStack.push(enPassant);
//	}
//	inline void StoreCRights(const CRightsFlags& cRights) {
//		PlyHistory& ply = mHistoryStack.top();  // TODO: try awoid this.
//		if (ply.storageFlags & StorageFlags::CRightsFlag != StorageFlags::None) throw MoveStackDoubleIrrevStore();
//		ply.storageFlags = ply.storageFlags | StorageFlags::CRightsFlag;
//		mCRightsStack.push(cRights);
//	}
//};
//
