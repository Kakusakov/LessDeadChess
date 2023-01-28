#pragma once
#include "Defines.h"
#include "Board.h"
#include "Position.h"
#include "FastStack.h"
#include <algorithm>


enum class MoveFlags : BYTE {
	Speñ0Flag = 1 << 0,
	Spec1Flag = 1 << 1,
	CaptureFlag = 1 << 2,
	PromoFlag = 1 << 3,

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
inline MoveFlags operator~(MoveFlags a) {
	return static_cast<MoveFlags>(~static_cast<BYTE>(a));
}
inline MoveFlags operator&(MoveFlags a, MoveFlags b) {
	return static_cast<MoveFlags>(static_cast<BYTE>(a) & static_cast<BYTE>(b));
}
inline MoveFlags operator|(MoveFlags a, MoveFlags b) {
	return static_cast<MoveFlags>(static_cast<BYTE>(a) | static_cast<BYTE>(b));
}
inline MoveFlags operator^(MoveFlags a, MoveFlags b) {
	return static_cast<MoveFlags>(static_cast<BYTE>(a) ^ static_cast<BYTE>(b));
}
inline MoveFlags& operator&= (MoveFlags& a, MoveFlags b) {
	a = a & b;
	return a;
}
inline MoveFlags& operator|= (MoveFlags& a, MoveFlags b) {
	a = a | b;
	return a;
}
inline MoveFlags& operator^= (MoveFlags& a, MoveFlags b) {
	a = a ^ b;
	return a;
}

enum class StorageFlags : BYTE {
	None = 0,
	CaptureFlag = 1 << 0,
	EnPassantFlag = 1 << 1,
	CRightsFlag = 1 << 2,
	HMClockFlag = 1 << 3
};
inline StorageFlags operator~(StorageFlags a) {
	return static_cast<StorageFlags>(~static_cast<BYTE>(a));
}
inline StorageFlags operator&(StorageFlags a, StorageFlags b) {
	return static_cast<StorageFlags>(static_cast<BYTE>(a) & static_cast<BYTE>(b));
}
inline StorageFlags operator|(StorageFlags a, StorageFlags b) {
	return static_cast<StorageFlags>(static_cast<BYTE>(a) | static_cast<BYTE>(b));
}
inline StorageFlags operator^(StorageFlags a, StorageFlags b) {
	return static_cast<StorageFlags>(static_cast<BYTE>(a) ^ static_cast<BYTE>(b));
}
inline StorageFlags& operator&= (StorageFlags& a, StorageFlags b) {
	a = a & b;
	return a;
}
inline StorageFlags& operator|= (StorageFlags& a, StorageFlags b) {
	a = a | b;
	return a;
}
inline StorageFlags& operator^= (StorageFlags& a, StorageFlags b) {
	a = a ^ b;
	return a;
}

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
	Draw50Moves,
	DrawInsuffMat
	// ThreefoldRep
};

class Move {
private:
	WORD mData;
public:
	Move() { mData = 0; }
	Move(Square from, Square to, MoveFlags flags) {
		mData = (((BYTE)flags & 0xf) << 12) | ((from & 0x3f) << 6) | (to & 0x3f);
	}
	inline Square getTo() const { return (Square)(mData & 0x3f); }
	inline Square getFrom() const { return (Square)((mData >> 6) & 0x3f); }
	inline MoveFlags getFlags() const { return (MoveFlags)((mData >> 12) & 0x0f); }
	inline WORD getRawData() const { return mData; }
};

struct PlyHistory {
	BYTE moveCount = 0;
	StorageFlags storageFlags = StorageFlags::None;
	MoveGenerationStage generationStage = (MoveGenerationStage)0;
};

template<size_t maxPly>
class MoveGen {
private:
	static constexpr size_t historyStackCapacity = maxPly;
	static constexpr size_t moveStackCapacity = maxPly * 64;
	static constexpr size_t pieceStackCapacity = std::min(maxPly, (size_t)32);
	static constexpr size_t hmClockStackCapacity = std::min(maxPly, (size_t)120);
	static constexpr size_t enPassantStackCapacity = std::min(maxPly, (size_t)16);
	static constexpr size_t ñRightsStackCapacity = std::min(maxPly, (size_t)4);

	Position mPos;
	FastStack<PlyHistory, historyStackCapacity> mHistoryStack = {};
	FastStack<Move, moveStackCapacity> mMoveStack = {};
	FastStack<Piece, pieceStackCapacity> mCaptureStack = {};
	FastStack<BYTE, hmClockStackCapacity> mHMClockStack = {};
	FastStack<EnPassant, enPassantStackCapacity> mEPStack = {};
	FastStack<CRightsFlags, ñRightsStackCapacity > mCRightsStack = {};

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
			generatePawnMovesToBuffer(); break;
		case MoveGenerationStage::RookLikeStage:
			generateRooklikeMovesToBuffer(); break;
		case MoveGenerationStage::BishopLikeStage:
			generateBishoplikeMovesToBuffer(); break;
		case MoveGenerationStage::KnightStage:
			generateKnightMovesToBuffer(); break;
		case MoveGenerationStage::KingStage:
			generateKingMovesToBuffer(); break;
		default: throw "invalid stage";  // TODO: contract this.
		}
		stage = (MoveGenerationStage)(((BYTE)stage) + 1);
		return true;
	}

	bool TryMakeValidMoveOrFinish();

	StorageFlags makeMove(const Move move);
	void unmakeMove(const Move move, const StorageFlags storageFlags);
public:
	inline MoveGen(const Position pos) : mPos(pos) { }
	inline const Position& getPosition() const { return mPos; }
	inline void resetRoot(Position root) {
		mPos = root;
		mHistoryStack.pop(mHistoryStack.maxPly());
		mMoveStack.pop(mMoveStack.maxPly());
		mCaptureStack.pop(mCaptureStack.maxPly());
		mHMClockStack.pop(mHMClockStack.maxPly());
		mEPStack.pop(mEPStack.maxPly());
		mCRightsStack.pop(mCRightsStack.maxPly());
	}
	inline void abandonMoves() {
		PlyHistory generationData = mHistoryStack.top();
		mHistoryStack.pop();
		unmakeMove(mMoveStack.top(), generationData.storageFlags);
		mMoveStack.pop(generationData.moveCount);
	}
	GeneratedNodeResult generateMoves();
	bool tryApplyNextMoveOrFinish();
	std::vector<Position> collectMovesAndFinish();
};

template<size_t maxPly>
bool MoveGen<maxPly>::TryMakeValidMoveOrFinish() {
	PlyHistory generationData = mHistoryStack.top();
	mHistoryStack.pop();
	const size_t lowerLengthBound = mMoveStack.maxPly() - generationData.moveCount;
	do {
		while (mMoveStack.maxPly() > lowerLengthBound) {
			Move move = mMoveStack.top();
			StorageFlags storageFlags = makeMove(move);
			if (mPos.board.getAttacksToColoredKing((Color)((mPos.turn + 1) & 1))) {
				unmakeMove(move, storageFlags);
				mMoveStack.pop();
				continue;
			}
			generationData.moveCount = mMoveStack.maxPly() - lowerLengthBound;
			generationData.storageFlags = storageFlags;
			mHistoryStack.push(generationData);
			return true;
		}
	} while (tryGenerateNextStageMoves(generationData.generationStage));
	return false;
}

template<size_t maxPly>
bool MoveGen<maxPly>::tryApplyNextMoveOrFinish() {
	PlyHistory generationData = mHistoryStack.top();
	mHistoryStack.pop();
#if defined DEBUG
	if (generationData.moveCount < 1) {
		throw std::logic_error("something went wrong");
	}
#endif
	unmakeMove(mMoveStack.top(), generationData.storageFlags);
	mMoveStack.pop();
	--generationData.moveCount;
	mHistoryStack.push(generationData);
	return TryMakeValidMoveOrFinish();
}

template<size_t maxPly>
GeneratedNodeResult MoveGen<maxPly>::generateMoves() {
	if (!(mPos.board.getBB(Board::PieceBB::Queen)) && !(mPos.board.getBB(Board::PieceBB::Rook))) {
		const U64 minorPieces = mPos.board.getBB(Board::PieceBB::Bishop)
			| mPos.board.getBB(Board::PieceBB::Knight);
		if (!(minorPieces & (minorPieces - 1))) return GeneratedNodeResult::DrawInsuffMat;
	}

	PlyHistory generationData = PlyHistory();
	const size_t initial_leghth = mMoveStack.maxPly();
	const U64 checkingPieces = mPos.board.getAttacksToColoredKing(mPos.turn);
	if (checkingPieces & (checkingPieces - 1)) {  // if double check
		generateKingMovesToBuffer();
		generationData.generationStage = MoveGenerationStage::FinalStage;
	}
	else {
		if (!tryGenerateNextStageMoves(generationData.generationStage)) throw "something went wrong";
		// TODO: remove the throw after debuging.
	}
	generationData.moveCount = mMoveStack.maxPly() - initial_leghth;
	mHistoryStack.push(generationData);

	if (TryMakeValidMoveOrFinish()) {
		if (mPos.hmClock >= 100) {
			abandonMoves();
			return GeneratedNodeResult::Draw50Moves;
		}
		return GeneratedNodeResult::None;
	}
	return checkingPieces ? GeneratedNodeResult::Checkmate : GeneratedNodeResult::Stalemate;
}

template<size_t maxPly>
void MoveGen<maxPly>::generatePawnMovesToBuffer()
{
	Color opponent = (Color)((mPos.turn + 1) & 1);
	U64 opponentBB = mPos.board.getColor(opponent);
	U64 occ = mPos.board.getOccupance();
	U64 pawnBB = mPos.board.getColoredPieces(mPos.turn, Piece::Pawn);
	// pawn attacks
	{
		U64 promoters = pawnBB & (mPos.turn == Color::White ? rank7 : rank2);
		while (promoters) {
			Square promoterSq = bitScanForward(promoters);
			U64 promotionAttacks = pawnAttacks[mPos.turn][promoterSq] & opponentBB;
			while (promotionAttacks) {
				Square to = bitScanForward(promotionAttacks);
				mMoveStack.push(Move(promoterSq, to, MoveFlags::BishopPromoCapture));
				mMoveStack.push(Move(promoterSq, to, MoveFlags::KnightPromoCapture));
				mMoveStack.push(Move(promoterSq, to, MoveFlags::RookPromoCapture));
				mMoveStack.push(Move(promoterSq, to, MoveFlags::QueenPromoCapture));
				promotionAttacks &= promotionAttacks - 1;
			}
			promoters &= promoters - 1;
		}
		U64 nonPromoters = pawnBB & ~promoters;
		while (nonPromoters) {
			Square from = bitScanForward(nonPromoters);
			U64 attacks = pawnAttacks[mPos.turn][from] & opponentBB;
			while (attacks) {
				mMoveStack.push(Move(from, bitScanForward(attacks), MoveFlags::Capture));
				attacks &= attacks - 1;
			}
			nonPromoters &= nonPromoters - 1;
		}
		U64 epAttackers = pawnBB & epPerformers[mPos.turn][mPos.enPassant];
		while (nonPromoters) {
			U64 to = epMoveTargets[mPos.turn][mPos.enPassant];
			if (to & ~occ) {
				mMoveStack.push(
					Move(bitScanForward(epAttackers),
						bitScanForward(to),
						MoveFlags::EPCapture
					)
				);
			}
			nonPromoters &= nonPromoters - 1;
		}
	}

	// pawn pushes
	{
		U64 pawnPushes = singlePushTargets(pawnBB, ~occ, mPos.turn);
		U64 promotionPushes = pawnPushes & (mPos.turn == Color::White ? rank8 : rank1);
		pawnPushes &= ~promotionPushes;
		U64 dblPawnPushes = singlePushTargets(
			pawnPushes & (mPos.turn == Color::White ? rank3 : rank6), 
			~occ, 
			mPos.turn
		);
		int pushOffset = mPos.turn == Color::White ? -8 : 8;
		while (pawnPushes) {
			Square to = bitScanForward(pawnPushes);
			mMoveStack.push(Move((Square)(to + pushOffset), to, MoveFlags::Quiet));
			pawnPushes &= pawnPushes - 1;
		}
		while (dblPawnPushes) {
			Square to = bitScanForward(dblPawnPushes);
			mMoveStack.push(Move((Square)(to + pushOffset * 2), to, MoveFlags::DoublePawn));
			dblPawnPushes &= dblPawnPushes - 1;
		}
		while (promotionPushes) {
			Square to = bitScanForward(promotionPushes);
			Square from = (Square)(to + pushOffset);
			mMoveStack.push(Move(from, to, MoveFlags::BishopPromo));
			mMoveStack.push(Move(from, to, MoveFlags::KnightPromo));
			mMoveStack.push(Move(from, to, MoveFlags::RookPromo));
			mMoveStack.push(Move(from, to, MoveFlags::QueenPromo));
			promotionPushes &= promotionPushes - 1;
		}
	}
}

template<size_t maxPly>
void MoveGen<maxPly>::generateRooklikeMovesToBuffer()
{
	Color opponent = (Color)((mPos.turn + 1) & 1);
	U64 opponentBB = mPos.board.getColor(opponent);
	U64 occ = mPos.board.getOccupance();
	U64 rookLike = mPos.board.getRookLikeSliders() & mPos.board.getColor(mPos.turn);
	while (rookLike) {
		Square from = bitScanForward(rookLike);
		U64 attacks = rookAttacks(occ, from);
		U64 attackHits = attacks & opponentBB;
		while (attackHits) {
			mMoveStack.push(Move(from, bitScanForward(attackHits), MoveFlags::Capture));
			attackHits &= attackHits - 1;
		}
		U64 attackQuiets = attacks & ~occ;
		while (attackQuiets) {
			mMoveStack.push(Move(from, bitScanForward(attackQuiets), MoveFlags::Quiet));
			attackQuiets &= attackQuiets - 1;
		}
		rookLike &= rookLike - 1;
	}
}

template<size_t maxPly>
void MoveGen<maxPly>::generateBishoplikeMovesToBuffer()
{
	Color opponent = (Color)((mPos.turn + 1) & 1);
	U64 opponentBB = mPos.board.getColor(opponent);
	U64 occ = mPos.board.getOccupance();
	U64 bishopLike = mPos.board.getBishopLikeSliders() & mPos.board.getColor(mPos.turn);
	while (bishopLike) {
		Square from = bitScanForward(bishopLike);
		U64 attacks = bishopAttacks(occ, from);
		U64 attackHits = attacks & opponentBB;
		while (attackHits) {
			mMoveStack.push(Move(from, bitScanForward(attackHits), MoveFlags::Capture));
			attackHits &= attackHits - 1;
		}
		U64 attackQuiets = attacks & ~occ;
		while (attackQuiets) {
			mMoveStack.push(Move(from, bitScanForward(attackQuiets), MoveFlags::Quiet));
			attackQuiets &= attackQuiets - 1;
		}
		bishopLike &= bishopLike - 1;
	}
}

template<size_t maxPly>
void MoveGen<maxPly>::generateKingMovesToBuffer()
{
	Color opponent = (Color)((mPos.turn + 1) & 1);
	U64 opponentBB = mPos.board.getColor(opponent);
	U64 occ = mPos.board.getOccupance();
	Square kingSq = bitScanForward(mPos.board.getColoredKing(mPos.turn));
	U64 attacks = kingAttacks[kingSq] & opponentBB;
	while (attacks) {
		mMoveStack.push(Move(kingSq, bitScanForward(attacks), MoveFlags::Capture));
		attacks &= attacks - 1;
	}
	U64 quiets = kingAttacks[kingSq] & ~occ;
	while (quiets) {
		mMoveStack.push(Move(kingSq, bitScanForward(quiets), MoveFlags::Quiet));
		quiets &= quiets - 1;
	}

	if (mPos.board.getAttacksToColoredKing(mPos.turn) == 0) {
		if (mPos.turn == Color::White) {
			if (mPos.cRights & CRightsFlags::WhiteKing
				&& !mPos.board.isWhiteKingsideCastleObstructed()
			) {
				mMoveStack.push(Move((Square)4, (Square)6, MoveFlags::KCastle));
			}
			if (mPos.cRights & CRightsFlags::WhiteQueen
				&& !mPos.board.isWhiteQueensideCastleObstructed()
			) {
				mMoveStack.push(Move((Square)4, (Square)2, MoveFlags::QCastle));
			}
		}
		else {
			if (mPos.cRights & CRightsFlags::BlackKing
				&& !mPos.board.isBlackKingsideCastleObstructed()
			) {
				mMoveStack.push(Move((Square)60, (Square)62, MoveFlags::KCastle));
			}
			if (mPos.cRights & CRightsFlags::BlackQueen
				&& !mPos.board.isBlackQueensideCastleObstructed()
			) {
				mMoveStack.push(Move((Square)60, (Square)58, MoveFlags::QCastle));
			}
		}
	}
}

template<size_t maxPly>
void MoveGen<maxPly>::generateKnightMovesToBuffer()
{
	Color opponent = (Color)((mPos.turn + 1) & 1);
	U64 opponentBB = mPos.board.getColor(opponent);
	U64 occ = mPos.board.getOccupance();
	U64 knightBB = mPos.board.getColoredPieces(mPos.turn, Piece::Knight);
	while (knightBB) {
		Square knightSq = bitScanForward(knightBB);
		U64 attacks = knightAttacks[knightSq] & opponentBB;
		while (attacks) {
			mMoveStack.push(Move(knightSq, bitScanForward(attacks), MoveFlags::Capture));
			attacks &= attacks - 1;
		}
		U64 quiets = knightAttacks[knightSq] & ~occ;
		while (quiets) {
			mMoveStack.push(Move(knightSq, bitScanForward(quiets), MoveFlags::Quiet));
			quiets &= quiets - 1;
		}
		knightBB &= knightBB - 1;
	}
}

template<size_t maxPly>
std::vector<Position> MoveGen<maxPly>::collectMovesAndFinish() {
	std::vector<Position> ret = {};
	do {
		ret.push_back(mPos);
	} while (tryApplyNextMoveOrFinish());
	return ret;
}

template<size_t maxPly>
StorageFlags MoveGen<maxPly>::makeMove(const Move move)
{
	const U64 fromBB = C64(1) << move.getFrom();
	const U64 toBB = C64(1) << move.getTo();
	const MoveFlags flags = move.getFlags();
	StorageFlags storageFlags = StorageFlags::None;
	const Color opponent = (Color)((mPos.turn + 1) & 1);
	Piece piece;

	if (flags == MoveFlags::Quiet || flags == MoveFlags::Capture) {
		if (mPos.board.getColoredPieces(mPos.turn, Piece::Pawn) & fromBB) piece = Piece::Pawn;
		else if (mPos.board.getColoredPieces(mPos.turn, Piece::Bishop) & fromBB) piece = Piece::Bishop;
		else if (mPos.board.getColoredPieces(mPos.turn, Piece::Knight) & fromBB) piece = Piece::Knight;
		else if (mPos.board.getColoredPieces(mPos.turn, Piece::Rook) & fromBB) piece = Piece::Rook;
		else if (mPos.board.getColoredPieces(mPos.turn, Piece::Queen) & fromBB) piece = Piece::Queen;
		else piece = Piece::King;
	}
	else if (flags == MoveFlags::DoublePawn || flags == MoveFlags::EPCapture) {
		piece = Piece::Pawn;
	}
	else if (static_cast<BYTE>(flags & MoveFlags::PromoFlag)) {
		piece = Piece::Pawn;
		mPos.board.removeColoredPieces(mPos.turn, Piece::Pawn, fromBB);
		mPos.board.createColoredPieces(mPos.turn, (Piece)((static_cast<BYTE>(flags) & 0b11) + 1), toBB);
	}
	else if (flags == MoveFlags::KCastle) {
		piece = Piece::King;  // Important, castling is king's move.
		mPos.board.updateColoredPieces(mPos.turn, Piece::Rook, kCastlingRookUpdate[mPos.turn]);
	}
	else if (flags == MoveFlags::QCastle) {
		piece = Piece::King;  // Important, castling is king's move.
		mPos.board.updateColoredPieces(mPos.turn, Piece::Rook, qCastlingRookUpdate[mPos.turn]);
	}
	else throw "Missing case";  // Maybe remove after debugging.

	if (static_cast<BYTE>(flags & MoveFlags::CaptureFlag)) {
		U64 atkBB;
		if (flags != MoveFlags::EPCapture) atkBB = toBB;
		else atkBB = epAttackTargets[mPos.turn][mPos.enPassant];
		Piece captured;
		if (mPos.board.getColoredPieces(opponent, Piece::Pawn) & atkBB) captured = Piece::Pawn;
		else if (mPos.board.getColoredPieces(opponent, Piece::Bishop) & atkBB) captured = Piece::Bishop;
		else if (mPos.board.getColoredPieces(opponent, Piece::Knight) & atkBB) captured = Piece::Knight;
		else if (mPos.board.getColoredPieces(opponent, Piece::Rook) & atkBB) captured = Piece::Rook;
		else captured = Piece::Queen;

		mCaptureStack.push(captured);
		storageFlags |= StorageFlags::CaptureFlag;
		mPos.board.removeColoredPieces(opponent, captured, atkBB);
	}

	if (!(static_cast<BYTE>(flags & MoveFlags::PromoFlag))) {
		mPos.board.updateColoredPieces(mPos.turn, piece, fromBB | toBB);
	}

	if ((static_cast<BYTE>(flags & MoveFlags::CaptureFlag))
		|| (piece == Piece::Pawn)) {
		mHMClockStack.push(mPos.hmClock);
		storageFlags |= StorageFlags::HMClockFlag;
		mPos.hmClock = 0;
	}
	else ++mPos.hmClock;

	if (mPos.enPassant != EnPassant::None) {
		mEPStack.push(mPos.enPassant);
		storageFlags |= StorageFlags::EnPassantFlag;
	}
	mPos.enPassant = flags == MoveFlags::DoublePawn ? (EnPassant)(move.getFrom() & 0x7) : EnPassant::None;

	const CRightsFlags qRookRightsBreak = mPos.turn == Color::White
		? CRightsFlags::WhiteQueen : CRightsFlags::BlackQueen;
	const CRightsFlags kRookRightsBreak = mPos.turn == Color::White
		? CRightsFlags::WhiteKing : CRightsFlags::BlackKing;
	const CRightsFlags kingRightsBreak = (CRightsFlags)(qRookRightsBreak | kRookRightsBreak);
	const U64 qRookInitialPos = mPos.turn == Color::White
		? C64(0x1) : C64(0x100000000000000);
	const U64 kRookInitialPos = mPos.turn == Color::White
		? C64(0x80) : C64(0x8000000000000000);

	if ((piece == Piece::King) && (mPos.cRights & kingRightsBreak)) {
		mCRightsStack.push(mPos.cRights);
		storageFlags = storageFlags | StorageFlags::CRightsFlag;
		mPos.cRights = (CRightsFlags)(mPos.cRights & ~kingRightsBreak);
	}
	else if (piece == Piece::Rook) {
		if ((fromBB & qRookInitialPos) && (mPos.cRights & qRookRightsBreak)) {
			mCRightsStack.push(mPos.cRights);
			storageFlags = storageFlags | StorageFlags::CRightsFlag;
			mPos.cRights = (CRightsFlags)(mPos.cRights & ~qRookRightsBreak);
		}
		else if ((fromBB & kRookInitialPos) && (mPos.cRights & kRookRightsBreak)) {
			mCRightsStack.push(mPos.cRights);
			storageFlags = storageFlags | StorageFlags::CRightsFlag;
			mPos.cRights = (CRightsFlags)(mPos.cRights & ~kRookRightsBreak);
		}
	}

	mPos.turn = opponent;

	return storageFlags;
}

template<size_t maxPly>
void MoveGen<maxPly>::unmakeMove(const Move move, const StorageFlags storageFlags)
{
	U64 fromBB = C64(1) << move.getFrom();
	U64 toBB = C64(1) << move.getTo();
	MoveFlags flags = move.getFlags();
	Color opponent = mPos.turn;
	mPos.turn = (Color)((mPos.turn + 1) & 1);
	Piece piece;

	if (static_cast<BYTE>(storageFlags) & static_cast<BYTE>(StorageFlags::CRightsFlag)) {
		mPos.cRights = mCRightsStack.top();
		mCRightsStack.pop();
	}
	if (static_cast<BYTE>(storageFlags) & static_cast<BYTE>(StorageFlags::EnPassantFlag)) {
		mPos.enPassant = mEPStack.top();
		mEPStack.pop();
	}
	else mPos.enPassant = EnPassant::None;
	if (static_cast<BYTE>(storageFlags) & static_cast<BYTE>(StorageFlags::HMClockFlag)) {
		mPos.hmClock = mHMClockStack.top();
		mHMClockStack.pop();
	}
	else --mPos.hmClock;

	if (flags == MoveFlags::Quiet || flags == MoveFlags::Capture) {
		if (mPos.board.getColoredPieces(mPos.turn, Piece::Pawn) & toBB) piece = Piece::Pawn;
		else if (mPos.board.getColoredPieces(mPos.turn, Piece::Bishop) & toBB) piece = Piece::Bishop;
		else if (mPos.board.getColoredPieces(mPos.turn, Piece::Knight) & toBB) piece = Piece::Knight;
		else if (mPos.board.getColoredPieces(mPos.turn, Piece::Rook) & toBB) piece = Piece::Rook;
		else if (mPos.board.getColoredPieces(mPos.turn, Piece::Queen) & toBB) piece = Piece::Queen;
		else piece = Piece::King;
	}
	else if (flags == MoveFlags::DoublePawn || flags == MoveFlags::EPCapture) {
		piece = Piece::Pawn;
	}
	else if (static_cast<BYTE>(flags) & static_cast<BYTE>(MoveFlags::PromoFlag)) {
		piece = Piece::Pawn;
		mPos.board.removeColoredPieces(mPos.turn, (Piece)((static_cast<BYTE>(flags) & 0b11) + 1), toBB);
		mPos.board.createColoredPieces(mPos.turn, Piece::Pawn, fromBB);
	}
	else if (flags == MoveFlags::KCastle) {
		piece = Piece::King;  // Important, castling is king's move.
		mPos.board.updateColoredPieces(mPos.turn, Piece::Rook, kCastlingRookUpdate[mPos.turn]);
	}
	else if (flags == MoveFlags::QCastle) {
		piece = Piece::King;  // Important, castling is king's move.
		mPos.board.updateColoredPieces(mPos.turn, Piece::Rook, qCastlingRookUpdate[mPos.turn]);
	}
	else throw "Missing case";  // Maybe remove after debugging.

	if (!(static_cast<BYTE>(flags) & static_cast<BYTE>(MoveFlags::PromoFlag))) {
		mPos.board.updateColoredPieces(mPos.turn, piece, fromBB | toBB);
	}

	if (static_cast<BYTE>(storageFlags) & static_cast<BYTE>(StorageFlags::CaptureFlag)) {
		U64 atkBB;
		if (flags != MoveFlags::EPCapture) atkBB = toBB;
		else atkBB = epAttackTargets[mPos.turn][mPos.enPassant];
		mPos.board.createColoredPieces(opponent, mCaptureStack.top(), atkBB);
		mCaptureStack.pop();
	}
}
