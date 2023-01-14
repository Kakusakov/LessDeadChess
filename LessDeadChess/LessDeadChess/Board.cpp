#include "Board.h"
#if defined DEBUG
#include <iostream>
#endif

U64 fillUpAttacks[8][64];  // 4 KByte
U64 aFileAttacks[8][64];  // 4 KByte

U64 diagonalMaskEx[64];  // 512 Byte
U64 antidiagMaskEx[64];  // 512 Byte
U64 rankMaskEx[64];  // 512 Byte

U64 kingAttacks[64];  // 512 Byte
U64 knightAttacks[64];  // 512 Byte
U64 pawnAttacks[2][64];  // 1 KByte
U64 pawnPushes[2][64];  // 1 KByte

U64 epMoveTargets[2][8];  // 128 Byte
U64 epAttackTargets[2][8];  // 128 Byte

U64 epPerformers[2][9];  // 144 Byte


const int index64[64] = {
	0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};  // 256 Byte

void initializeKGLookups() {
#if defined DEBUG
	std::cout << "ENGINE >> BOARD >> initializing kindergarten lookups..." << std::endl;
#endif
	for (U64 occ6bit = 0; occ6bit < 64; ++occ6bit) {
		for (int sq = 0; sq < 8; ++sq) {
			U64 rooks = C64(1) << sq;
			U64 empty = ~(occ6bit << 1);
			U64 firstRankAttacks = eastAttacks(rooks, empty) | westAttacks(rooks, empty);

			fillUpAttacks[sq][occ6bit] = nortOccluded(firstRankAttacks, universeSet);
			aFileAttacks[sq][occ6bit] = eastOccluded(
				((firstRankAttacks * diaA1H8) >> 7) & aFile, 
				universeSet
			);
#if defined DEBUG_VERBOSE
			std::cout << "fillUpAttacks[" +
				std::to_string(sq) + "][" +
				std::to_string(occ6bit) + "] = {\n" +
				BBToString(fillUpAttacks[sq][occ6bit]) + "}\n" << std::endl;
			std::cout << "aFileAttacks[" +
				std::to_string(sq) + "][" +
				std::to_string(occ6bit) + "] = {\n" +
				BBToString(aFileAttacks[sq][occ6bit]) + "}\n" << std::endl;
#endif
		}
	}
}

void initializeLineMasks() {
#if defined DEBUG
	std::cout << "ENGINE >> BOARD >> initializing line mask lookups..." << std::endl;
#endif
	for (int sq = 0; sq < 64; ++sq) {
		U64 single = C64(1) << sq;
		rankMaskEx[sq] = rankMask((Square)sq) ^ single;
		diagonalMaskEx[sq] = diagonalMask((Square)sq) ^ single;
		antidiagMaskEx[sq] = antiDiagMask((Square)sq) ^ single;
#if defined DEBUG_VERBOSE
	std::cout << "rankMaskEx[" +
		std::to_string(sq) + "] = {\n" +
		BBToString(rankMaskEx[sq]) + "}\n" << std::endl;
	std::cout << "diagonalMaskEx[" +
		std::to_string(sq) + "] = {\n" +
		BBToString(diagonalMaskEx[sq]) + "}\n" << std::endl;
	std::cout << "antidiagMaskEx[" +
		std::to_string(sq) + "] = {\n" +
		BBToString(antidiagMaskEx[sq]) + "}\n" << std::endl;
#endif
	}
}

void initializeSimpleAttackLookups() {
#if defined DEBUG
	std::cout << "ENGINE >> BOARD >> initializing simple attack lookups..." << std::endl;
#endif
	for (int sq = 0; sq < 64; ++sq) {
		U64 single = C64(1) << sq;
		kingAttacks[sq] = kingAttackSet(single);
		knightAttacks[sq] = knightAttackSet(single);
#if defined DEBUG_VERBOSE
		std::cout << "kingAttacks[" +
			std::to_string(sq) + "] = {\n" +
			BBToString(kingAttacks[sq]) + "}\n" << std::endl;
		std::cout << "knightAttacks[" +
			std::to_string(sq) + "] = {\n" +
			BBToString(knightAttacks[sq]) + "}\n" << std::endl;
#endif
	}
}

void inititalizePawnAttacks() {
#if defined DEBUG
	std::cout << "ENGINE >> BOARD >> initializing pawn attack lookups..." << std::endl;
#endif
	for (int sq = 0; sq < 64; ++sq) {
		U64 single = C64(1) << sq;
		pawnAttacks[Color::White][sq] = wPawnWestAttackSet(single) | wPawnEastAttackSet(single);
		pawnAttacks[Color::Black][sq] = bPawnWestAttackSet(single) | bPawnEastAttackSet(single);
#if defined DEBUG_VERBOSE
	std::cout << "pawnAttacks[" +
		std::to_string(Color::White) + "][" +
		std::to_string(sq) + "] = {\n" +
		BBToString(pawnAttacks[Color::White][sq]) + "}\n" << std::endl;
	std::cout << "pawnAttacks[" +
		std::to_string(Color::Black) + "][" +
		std::to_string(sq) + "] = {\n" +
		BBToString(pawnAttacks[Color::Black][sq]) + "}\n" << std::endl;
#endif
	}
}

void inititalizeEpAttackTargets() {
#if defined DEBUG
	std::cout << "ENGINE >> BOARD >> initializing en passant attack target lookups..." << std::endl;
#endif
	for (int epCol = EnPassant::AFile; epCol <= EnPassant::HFile; ++epCol) {
		epAttackTargets[Color::White][epCol] = fileMask((Square)epCol) & rank5;
		epAttackTargets[Color::Black][epCol] = fileMask((Square)epCol) & rank4;
#if defined DEBUG_VERBOSE
		std::cout << "epAttackTargets[" +
			std::to_string(Color::White) + "][" +
			std::to_string(epCol) + "] = {\n" +
			BBToString(epAttackTargets[Color::White][epCol]) + "}\n" << std::endl;
		std::cout << "epAttackTargets[" +
			std::to_string(Color::Black) + "][" +
			std::to_string(epCol) + "] = {\n" +
			BBToString(epAttackTargets[Color::White][epCol]) + "}\n" << std::endl;
#endif
	}
}

void inititalizeEpMoveTargets() {
#if defined DEBUG
	std::cout << "ENGINE >> BOARD >> initializing en passant move target lookups..." << std::endl;
#endif
	for (int epCol = EnPassant::AFile; epCol <= EnPassant::HFile; ++epCol) {
		epMoveTargets[Color::White][epCol] = fileMask((Square)epCol) & rank6;
		epMoveTargets[Color::Black][epCol] = fileMask((Square)epCol) & rank3;
#if defined DEBUG_VERBOSE
	std::cout << "epTargets[" +
		std::to_string(Color::White) + "][" +
		std::to_string(epCol) + "] = {\n" +
		BBToString(epTargets[Color::White][epCol]) + "}\n" << std::endl;
	std::cout << "epTargets[" +
		std::to_string(Color::Black) + "][" +
		std::to_string(epCol) + "] = {\n" +
		BBToString(epTargets[Color::White][epCol]) + "}\n" << std::endl;
#endif
	}
}

void inititalizeEpPerformers() {
#if defined DEBUG
	std::cout << "ENGINE >> BOARD >> initializing en passant performers lookups..." << std::endl;
#endif
	for (int epCol = EnPassant::AFile; epCol <= EnPassant::HFile; ++epCol) {
		U64 wEpBB = fileMask((Square)epCol) & rank5;
		epPerformers[Color::White][epCol] = eastOne(wEpBB) | westOne(wEpBB);
		U64 bEpBB = fileMask((Square)epCol) & rank4;
		epPerformers[Color::Black][epCol] = eastOne(bEpBB) | westOne(bEpBB);
#if defined DEBUG_VERBOSE
		std::cout << "epPerformers[" +
			std::to_string(Color::White) + "][" +
			std::to_string(epCol) + "] = {\n" +
			BBToString(epPerformers[Color::White][epCol]) + "}\n" << std::endl;
		std::cout << "epPerformers[" +
			std::to_string(Color::Black) + "][" +
			std::to_string(epCol) + "] = {\n" +
			BBToString(epPerformers[Color::White][epCol]) + "}\n" << std::endl;
#endif
	}
	epPerformers[Color::White][EnPassant::None] = emptySet;
	epPerformers[Color::Black][EnPassant::None] = emptySet;
#if defined DEBUG_VERBOSE
	std::cout << "epPerformers[" +
		std::to_string(Color::White) + "][" +
		std::to_string(EnPassant::None) + "] = {\n" +
		BBToString(epPerformers[Color::White][EnPassant::None]) + "}\n" << std::endl;
	std::cout << "epPerformers[" +
		std::to_string(Color::Black) + "][" +
		std::to_string(EnPassant::None) + "] = {\n" +
		BBToString(epPerformers[Color::White][EnPassant::None]) + "}\n" << std::endl;
#endif
}

//void inititalizePawnPushes() {
//#if defined DEBUG
//	std::cout << "ENGINE >> BOARD >> initializing pawn pushes lookups..." << std::endl;
//#endif
//	for (int sq = 0; sq < 64; ++sq) {
//		U64 single = C64(1) << sq;
//		pawnPushes[Color::White][sq] = singlePushTargets(single, universeSet, Color::White);
//		pawnPushes[Color::Black][sq] = singlePushTargets(single, universeSet, Color::Black);
//#if defined DEBUG_VERBOSE
//	std::cout << "pawnPushes[" +
//		std::to_string(Color::White) + "][" +
//		std::to_string(sq) + "] = {\n" +
//		BBToString(pawnPushes[Color::White][sq]) + "}\n" << std::endl;
//	std::cout << "pawnPushes[" +
//		std::to_string(Color::Black) + "][" +
//		std::to_string(sq) + "] = {\n" +
//		BBToString(pawnPushes[Color::Black][sq]) + "}\n" << std::endl;
//#endif
//	}
//}

void initalizeBoardClass() {
#if defined DEBUG
	std::cout << "ENGINE >> initializing board class..." << std::endl;
#endif
	initializeKGLookups();
	initializeLineMasks();
	initializeSimpleAttackLookups();
	//inititalizePawnPushes();
	inititalizePawnAttacks();
	inititalizeEpMoveTargets();
	inititalizeEpPerformers();
#if defined DEBUG
	std::cout << "ENGINE >> finished initializing board class" << std::endl;
#endif
}

Square bitScanForward(U64 bb) {
	return (Square)index64[((bb ^ (bb - 1)) * C64(0x03f79d71b4cb0a89)) >> 58];
}

int popCount(U64 bb) {
	int count = 0;
	while (bb) {
		count++;
		bb &= bb - 1;
	}
	return count;
}

U64 rankMask(Square sq) {
	return rank1 << (sq & 56); 
}

U64 fileMask(Square sq) {
	return aFile << (sq & 7);
}

U64 diagonalMask(Square sq) {
	int diag = 8 * (sq & 7) - (sq & 56);
	int nort = -diag & (diag >> 31);
	int sout = diag & (-diag >> 31);
	return (diaA1H8 >> sout) << nort;
}

U64 antiDiagMask(Square sq) {
	int diag = 56 - 8 * (sq & 7) - (sq & 56);
	int nort = -diag & (diag >> 31);
	int sout = diag & (-diag >> 31);
	return (diaH1A8 >> sout) << nort;
}

U64 eastAttacks(U64 rooks, U64 empty) {
	empty = empty & notAFile; // make A-File all occupied, to consider H-A-wraps after shift
	rooks |= empty & (rooks << 1); // 1. fill
	rooks |= empty & (rooks << 1); // 2. fill
	rooks |= empty & (rooks << 1); // 3. fill
	rooks |= empty & (rooks << 1); // 4. fill
	rooks |= empty & (rooks << 1); // 5. fill
	rooks |= empty & (rooks << 1); // 6. fill
	return notAFile & (rooks << 1);
}

U64 westAttacks(U64 rooks, U64 empty) {
	empty = empty & notHFile;
	rooks |= empty & (rooks >> 1);
	rooks |= empty & (rooks >> 1);
	rooks |= empty & (rooks >> 1);
	rooks |= empty & (rooks >> 1);
	rooks |= empty & (rooks >> 1);
	rooks |= empty & (rooks >> 1);
	return notHFile & (rooks >> 1);
}

U64 nortAttacks(U64 rooks, U64 empty) {
	rooks |= empty & (rooks << 8);
	rooks |= empty & (rooks << 8);
	rooks |= empty & (rooks << 8);
	rooks |= empty & (rooks << 8);
	rooks |= empty & (rooks << 8);
	rooks |= empty & (rooks << 8);
	return rooks << 8;
}

U64 soutAttacks(U64 rooks, U64 empty) {
	rooks |= empty & (rooks >> 8);
	rooks |= empty & (rooks >> 8);
	rooks |= empty & (rooks >> 8);
	rooks |= empty & (rooks >> 8);
	rooks |= empty & (rooks >> 8);
	rooks |= empty & (rooks >> 8);
	return rooks >> 8;
}

U64 eastOccluded(U64 rooks, U64 empty) {
	empty = empty & notAFile; // make A-File all occupied, to consider H-A-wraps after shift
	rooks |= empty & (rooks << 1);
	empty = empty & (empty << 1);
	rooks |= empty & (rooks << 2);
	empty = empty & (empty << 2);
	rooks |= empty & (rooks << 4);
	return rooks;
}

U64 nortOccluded(U64 rooks, U64 empty) {
	rooks |= empty & (rooks << 8);
	empty = empty & (empty << 8);
	rooks |= empty & (rooks << 16);
	empty = empty & (empty << 16);
	rooks |= empty & (rooks << 32);
	return rooks;
}

U64 diagonalAttacks(U64 occ, Square sq) {
	occ = (diagonalMaskEx[sq] & occ) * bFile >> 58;
	return diagonalMaskEx[sq] & fillUpAttacks[sq & 7][occ];
}

U64 antiDiagAttacks(U64 occ, Square sq) {
	occ = (antidiagMaskEx[sq] & occ) * bFile >> 58;
	return antidiagMaskEx[sq] & fillUpAttacks[sq & 7][occ];
}

U64 rankAttacks(U64 occ, Square sq) {
	occ = (rankMaskEx[sq] & occ) * bFile >> 58;
	return rankMaskEx[sq] & fillUpAttacks[sq & 7][occ];
}

U64 fileAttacks(U64 occ, Square sq) {
	occ = aFile & (occ >> (sq & 7));
	occ = (diaC2H7 * occ) >> 58;
	return aFileAttacks[sq >> 3][occ] << (sq & 7);
}

U64 knightAttackSet(U64 knights) {
	U64 h1 = ((knights >> 1) & C64(0x7f7f7f7f7f7f7f7f)) 
		| ((knights << 1) & C64(0xfefefefefefefefe));
	U64 h2 = ((knights >> 2) & C64(0x3f3f3f3f3f3f3f3f)) 
		| ((knights << 2) & C64(0xfcfcfcfcfcfcfcfc));
	return (h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8);
}

U64 kingAttackSet(U64 kingSet) {
	U64 attacks = eastOne(kingSet) | westOne(kingSet);
	kingSet |= attacks;
	attacks |= nortOne(kingSet) | soutOne(kingSet);
	return attacks;
}

U64 singlePushTargets(U64 pawns, U64 empty, Color color) {
	return ((pawns << 8) >> (color << 4)) & empty;
}

U64 wPawnEastAttackSet(U64 wpawns) { return noEaOne(wpawns); }
U64 wPawnWestAttackSet(U64 wpawns) { return noWeOne(wpawns); }

U64 bPawnEastAttackSet(U64 bpawns) { return soEaOne(bpawns); }
U64 bPawnWestAttackSet(U64 bpawns) { return soWeOne(bpawns); }

std::string BBToString(U64 bb) {
	std::string ret = "";
	std::string row;
	for (int y = 0; y < 8; ++y) {
		row = "";
		for (int x = 0; x < 8; ++x, bb >>= 1) {
			if (bb & 1) row += "_ ";
			else row += "1 ";
		}
		ret = row + "\n" + ret;
	}
	return ret;
}
