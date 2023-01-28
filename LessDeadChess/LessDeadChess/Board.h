#pragma once
#include "Defines.h"
#include <string>
#include <vector>

enum Piece : BYTE {
	Pawn,
	Knight,
	Bishop,
	Rook,
	Queen,
	King
};

enum Color : BYTE {
	White,
	Black
};
inline Color operator~(Color a) {
	return static_cast<Color>((static_cast<BYTE>(a) + 1) & 1);
}

enum CRightsFlags : BYTE {
	WhiteKing = 1 << 0,
	WhiteQueen = 1 << 1,
	BlackKing = 1 << 2,
	BlackQueen = 1 << 3
};



enum EnPassant {
	// Careful when packing EnPassant, it must have at least 4 bits to represent the 'None' state.
	AFile = 0,
	BFile = 1,
	CFile = 2,
	DFile = 3,
	EFile = 4,
	FFile = 5,
	GFile = 6,
	HFile = 7,
	None = 8
};

enum Square {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

const U64 emptySet = C64(0);
const U64 universeSet = ~emptySet;
const U64 aFile = C64(0x101010101010101);
const U64 hFile = C64(0x8080808080808080);
const U64 notAFile = ~aFile;
const U64 notHFile = ~hFile;
const U64 diaA1H8 = C64(0x8040201008040201);
const U64 diaH1A8 = C64(0x0102040810204080);
const U64 diaC2H7 = C64(0x0080402010080400);
const U64 rank1 = C64(0xff);
const U64 rank2 = C64(0xff00);
const U64 rank3 = C64(0xff0000);
const U64 rank4 = C64(0xff000000);
const U64 rank5 = C64(0xff00000000);
const U64 rank6 = C64(0xff0000000000);
const U64 rank7 = C64(0xff000000000000);
const U64 rank8 = C64(0xff00000000000000);
const U64 bFile = C64(0x0202020202020202);

const U64 wQCastleObstructions = C64(0xE);
const U64 wKCastleObstructions = C64(0x60);
const U64 bQCastleObstructions = C64(0xE00000000000000);
const U64 bKCastleObstructions = C64(0x6000000000000000);

extern U64 fillUpAttacks[8][64];  // 4 KByte
extern U64 aFileAttacks[8][64];  // 4 KByte

extern U64 diagonalMaskEx[64];  // 512 Byte
extern U64 antidiagMaskEx[64];  // 512 Byte
extern U64 rankMaskEx[64];  // 512 Byte

extern U64 kingAttacks[64];  // 512 Byte
extern U64 knightAttacks[64];  // 512 Byte
extern U64 pawnAttacks[2][64];  // 1 KByte

extern U64 epMoveTargets[2][8];  // 128 Byte
extern U64 epAttackTargets[2][8];  // 128 Byte

extern U64 epPerformers[2][9];  // 144 Byte

extern U64 kCastlingRookUpdate[2];  // 16 Byte
extern U64 qCastlingRookUpdate[2];  // 16 Byte


//extern U64 pawnPushes[2][64];  // 1 KByte

std::string BBToString(U64 bb);
std::vector<Square> serializeBB(U64 bb);

Square bitScanForward(U64 bb);
int popCount(U64 bb);

U64 flipVertical(U64 x);
U64 mirrorHorizontal(U64 x);

U64 rankMask(Square sq);
U64 fileMask(Square sq);
U64 diagonalMask(Square sq);
U64 antiDiagMask(Square sq);

U64 eastAttacks(U64 rooks, U64 empty);
U64 westAttacks(U64 rooks, U64 empty);
U64 nortAttacks(U64 rooks, U64 empty);
U64 soutAttacks(U64 rooks, U64 empty);
U64 eastOccluded(U64 rooks, U64 empty);
U64 nortOccluded(U64 rooks, U64 empty);

inline U64 soutOne(U64 b) { return  b >> 8; }
inline U64 nortOne(U64 b) { return  b << 8; }
inline U64 eastOne(U64 b) { return (b << 1) & notAFile; }
inline U64 noEaOne(U64 b) { return (b << 9) & notAFile; }
inline U64 soEaOne(U64 b) { return (b >> 7) & notAFile; }
inline U64 westOne(U64 b) { return (b >> 1) & notHFile; }
inline U64 soWeOne(U64 b) { return (b >> 9) & notHFile; }
inline U64 noWeOne(U64 b) { return (b << 7) & notHFile; }

U64 rankAttacks(U64 occ, Square sq);
U64 fileAttacks(U64 occ, Square sq);
U64 diagonalAttacks(U64 occ, Square sq);
U64 antiDiagAttacks(U64 occ, Square sq);

inline U64 bishopAttacks(U64 occ, Square sq) { return diagonalAttacks(occ, sq) | antiDiagAttacks(occ, sq); }
inline U64 rookAttacks(U64 occ, Square sq) { return rankAttacks(occ, sq) | fileAttacks(occ, sq); }

U64 knightAttackSet(U64 knights);
U64 kingAttackSet(U64 kingSet);
U64 singlePushTargets(U64 pawns, U64 empty, Color color);

U64 wPawnEastAttackSet(U64 wpawns);
U64 wPawnWestAttackSet(U64 wpawns);
U64 bPawnEastAttackSet(U64 bpawns);
U64 bPawnWestAttackSet(U64 bpawns);

void initalizeBoardClass();

class Board
{
private:
	U64 mPieceBB[8] = { 0 };
public:
	enum PieceBB {
		White,
		Black,
		Pawn,
		Knight,
		Bishop,
		Rook,
		Queen,
		King
	};

	inline U64 getBB(PieceBB piece) const { return mPieceBB[piece]; }
	inline U64 getColor(Color color) const { return mPieceBB[color]; }
	inline U64 getPiece(Piece piece) const { return mPieceBB[piece + 2]; }
	inline U64 getColoredPieces(Color color, Piece piece) const { return mPieceBB[piece + 2] & mPieceBB[color]; }
	inline U64 getColoredKing(Color color) const { return mPieceBB[PieceBB::King] & mPieceBB[color]; }
	inline U64 getWhitePawns() const { return mPieceBB[PieceBB::Pawn] & mPieceBB[PieceBB::White]; }
	inline U64 getBlackPawns() const { return mPieceBB[PieceBB::Pawn] & mPieceBB[PieceBB::Black]; }
	inline U64 getBishopLikeSliders() const { return mPieceBB[PieceBB::Queen] | mPieceBB[PieceBB::Bishop]; }
	inline U64 getRookLikeSliders() const { return mPieceBB[PieceBB::Queen] | mPieceBB[PieceBB::Rook]; }
	inline U64 getOccupance() const { return mPieceBB[PieceBB::White] | mPieceBB[PieceBB::Black]; }
	inline U64 getEmpty() const { return ~getOccupance(); }
	inline U64 getAttacksTo(Square sq) const {
		const U64 occ = getOccupance();
		return (pawnAttacks[Color::White][sq] & getColoredPieces(Color::Black, Piece::Pawn))
			| (pawnAttacks[Color::Black][sq] & getColoredPieces(Color::White, Piece::Pawn))
			| (knightAttacks[sq] & mPieceBB[PieceBB::Knight])
			| (kingAttacks[sq] & mPieceBB[PieceBB::King])
			| (bishopAttacks(occ, sq) & (mPieceBB[PieceBB::Queen] | mPieceBB[PieceBB::Bishop]))
			| (rookAttacks(occ, sq) & (mPieceBB[PieceBB::Queen] | mPieceBB[PieceBB::Rook]));
	}
	inline U64 getAttacksToColoredKing(Color color) const {
		return getAttacksTo(bitScanForward(getColoredKing(color))) & getColor((Color)((color + 1) & 1));
	}
	inline void removeColoredPieces(Color color, Piece piece, U64 remove) {
		mPieceBB[piece + 2] &= ~remove;
		mPieceBB[color] &= ~remove;
	}
	inline void createColoredPieces(Color color, Piece piece, U64 create) {
		mPieceBB[piece + 2] |= create;
		mPieceBB[color] |= create;
	}
	inline void updateColoredPieces(Color color, Piece piece, U64 update) {
		mPieceBB[piece + 2] ^= update;
		mPieceBB[color] ^= update;
	}
	bool isWhiteKingsideCastleObstructed() const;
	bool isBlackKingsideCastleObstructed() const;
	bool isWhiteQueensideCastleObstructed() const;
	bool isBlackQueensideCastleObstructed() const;
};
