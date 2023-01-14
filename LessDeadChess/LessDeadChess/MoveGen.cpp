#include "MoveGen.h"


//PlyGenerationData MoveGen::GenerateMovesFromBoard() {
//	const size_t InitialBufferIdx = mMoveStack.length();
//	U64 checkingPieces = mPos.board.getAttacksTo(bitScanForward(mPos.board.getColoredKing(mPos.turn)));
//	if (checkingPieces & (checkingPieces - 1)) {  // if double check
//		GenerateKingMovesToBuffer();
//		return PlyGenerationData(mMoveStack.length() - InitialBufferIdx);
//	}
//	GeneratePawnMovesToBuffer();
//	GenerateKnightMovesToBuffer();
//	GenerateBishoplikeKingMovesToBuffer();
//	GenerateRooklikeMovesToBuffer();
//	GenerateKingMovesToBuffer();
//	// TODO: generate castles.
//	return PlyGenerationData(mMoveStack.length() - InitialBufferIdx);
//}

bool MoveGen::tryApplyNextMoveAndFinishOtherwise() {
	PlyGenerationData generationData = mHistoryStack.pop();
	if (generationData.moveCount < 1) throw "something went wrong";
	// TODO: remove throw after debuging.
	if (generationData.generationStage == MoveGenerationStage::FinalStage) {
		if (generationData.moveCount == 1) {
			unmakeMove(mMoveStack.pop(), generationData.storageFlags);
			return false;
		}
	}
	unmakeMove(mMoveStack.pop(), generationData.storageFlags);
	if (generationData.moveCount == 1) {
		const size_t initial_lenghth = mMoveStack.length();
		while (initial_lenghth == mMoveStack.length()) {
			if (!tryGenerateNextStageMoves(generationData.generationStage)) {
				return false;
			}
		}
	}

	generationData.storageFlags = makeMove(mMoveStack[mMoveStack.length() - 1]);
	--generationData.moveCount;
	mHistoryStack.push(generationData);
	return true;
}

GeneratedNodeResult MoveGen::generateMoves() {
	PlyGenerationData generationData = PlyGenerationData();
	const size_t initial_leghth = mMoveStack.length();
	U64 checkingPieces = mPos.board.getAttacksToColoredKing(mPos.turn);
	if (checkingPieces & (checkingPieces - 1)) {  // if double check
		generateKingMovesToBuffer();
		generationData.generationStage = MoveGenerationStage::FinalStage;
	}
	else {
		if (!tryGenerateNextStageMoves(generationData.generationStage)) throw "something went wrong";
		// TODO: remove the throw after debuging.
	}

	do {
		while (mMoveStack.length() > initial_leghth) {
			Move move = mMoveStack[mMoveStack.length() - 1];
			MoveStorageFlags storageFlags = makeMove(move);
			if (mPos.board.getAttacksToColoredKing((Color)((mPos.turn + 1) & 1))) {
				unmakeMove(mMoveStack.pop(), storageFlags);
				continue;
			}
			if (mPos.hmClock >= 100) return GeneratedNodeResult::Draw50Moves;
			generationData.moveCount = mMoveStack.length() - initial_leghth;
			generationData.storageFlags = storageFlags;
			mHistoryStack.push(generationData);
			return GeneratedNodeResult::None;
		}
	} while (tryGenerateNextStageMoves(generationData.generationStage));
	return checkingPieces ? GeneratedNodeResult::Checkmate : GeneratedNodeResult::Stalemate;
}

void MoveGen::generatePawnMovesToBuffer()
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
				mMoveStack.push(Move(promoterSq, to, MoveFlags::QueenPromoCapture));
				mMoveStack.push(Move(promoterSq, to, MoveFlags::RookPromoCapture));
				mMoveStack.push(Move(promoterSq, to, MoveFlags::KnightPromoCapture));
				mMoveStack.push(Move(promoterSq, to, MoveFlags::BishopPromoCapture));
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
		int pushOffset = mPos.turn == Color::White ? -8 : 8;
		while (pawnPushes) {
			Square to = bitScanForward(pawnPushes);
			mMoveStack.push(Move((Square)(to + pushOffset), to, MoveFlags::Quiet));
			pawnPushes &= pawnPushes - 1;
		}
		U64 dblPawnPushes = singlePushTargets(pawnPushes, ~occ, mPos.turn) 
			& (mPos.turn == Color::White ? rank4 : rank5);
		while (dblPawnPushes) {
			Square to = bitScanForward(dblPawnPushes);
			mMoveStack.push(Move((Square)(to + pushOffset * 2), to, MoveFlags::DoublePawn));
			dblPawnPushes &= dblPawnPushes - 1;
		}
		while (promotionPushes) {
			Square to = bitScanForward(promotionPushes);
			Square from = (Square)(to + pushOffset);
			mMoveStack.push(Move(from, to, MoveFlags::QueenPromo));
			mMoveStack.push(Move(from, to, MoveFlags::RookPromo));
			mMoveStack.push(Move(from, to, MoveFlags::KnightPromo));
			mMoveStack.push(Move(from, to, MoveFlags::BishopPromo));
			promotionPushes &= promotionPushes - 1;
		}
	}
}

void MoveGen::generateRooklikeMovesToBuffer()
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

void MoveGen::generateBishoplikeKingMovesToBuffer()
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

void MoveGen::generateKingMovesToBuffer()
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
}

void MoveGen::generateKnightMovesToBuffer()
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

MoveStorageFlags MoveGen::makeMove(const Move move)
{
	U64 fromBB = C64(1) << move.getFrom();
	U64 toBB = C64(1) << move.getTo();
	MoveFlags flags = move.getFlags();
	MoveStorageFlags storageFlags = MoveStorageFlags::None;
	Color opponent = (Color)((mPos.turn + 1) & 1);
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
	else if (flags & MoveFlags::PromoFlag) {
		piece = Piece::Pawn;
		mPos.board.removeColoredPieces(mPos.turn, Piece::Pawn, fromBB);
		mPos.board.createColoredPiece(mPos.turn, (Piece)((flags & 0b11) + 1), toBB);
	}
	else if (flags == MoveFlags::KCastle || flags == MoveFlags::QCastle) {
		piece = Piece::King;  // Important, castling is king's move.
		// TODO: implement castling.
	}
	else throw "Missing case";  // Maybe remove after debugging.

	if (!(flags & MoveFlags::PromoFlag)) {
		mPos.board.updateColoredPiece(mPos.turn, piece, fromBB | toBB);
	}

	if ((flags & MoveFlags::CaptureFlag)) {
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
		storageFlags = (MoveStorageFlags)(storageFlags | MoveStorageFlags::CaptureFlag);
		mPos.board.removeColoredPieces(opponent, captured, atkBB);
	}

	if ((flags & MoveFlags::CaptureFlag) || (piece == Piece::Pawn)) {
		mHMClockStack.push(mPos.hmClock);
		storageFlags = (MoveStorageFlags)(storageFlags | MoveStorageFlags::HMClockFlag);
		mPos.hmClock = 0;
	}
	else ++mPos.hmClock;

	if (mPos.enPassant != EnPassant::None) {
		mEPStack.push(mPos.enPassant);
		storageFlags = (MoveStorageFlags)(storageFlags | MoveStorageFlags::EnPassantFlag);
	}
	mPos.enPassant = flags == MoveFlags::DoublePawn ? (EnPassant)(move.getFrom() & 0x7) : EnPassant::None;

	// TODO: rook moves can also break castling rights.
	CRightsFlags kingMoveBrokenCRights = (CRightsFlags)(
		(CRightsFlags::WhiteKing | CRightsFlags::WhiteQueen) * (mPos.turn == Color::White ? 1 : 4)
	);
	if ((piece == Piece::King) && (mPos.cRights & kingMoveBrokenCRights)) {
		mCRightsStack.push(mPos.cRights);
		storageFlags = (MoveStorageFlags)(storageFlags | MoveStorageFlags::CRightsFlag);
		mPos.cRights = (CRightsFlags)(mPos.cRights & ~kingMoveBrokenCRights);
	}

	mPos.turn = opponent;

	return storageFlags;
}

void MoveGen::unmakeMove(const Move move, const MoveStorageFlags storageFlags)
{
	U64 fromBB = C64(1) << move.getFrom();
	U64 toBB = C64(1) << move.getTo();
	MoveFlags flags = move.getFlags();
	Color opponent = mPos.turn;
	mPos.turn = (Color)((mPos.turn + 1) & 1);
	Piece piece;

	if (storageFlags & MoveStorageFlags::CRightsFlag) mPos.cRights = mCRightsStack.pop();
	if (storageFlags & MoveStorageFlags::EnPassantFlag) mPos.enPassant = mEPStack.pop();
	if (storageFlags & MoveStorageFlags::HMClockFlag) mPos.hmClock = mHMClockStack.pop();

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
	else if (flags & MoveFlags::PromoFlag) {
		piece = Piece::Pawn;
		mPos.board.removeColoredPieces(mPos.turn, (Piece)((flags & 0b11) + 1), toBB);
		mPos.board.createColoredPiece(mPos.turn, Piece::Pawn, fromBB);
	}
	else if (flags == MoveFlags::KCastle || flags == MoveFlags::QCastle) {
		piece = Piece::King;  // Important, castling is king's move.
		// TODO: implement castling.
	}
	else throw "Missing case";  // Maybe remove after debugging.

	if (!(flags & MoveFlags::PromoFlag)) {
		mPos.board.updateColoredPiece(mPos.turn, piece, fromBB | toBB);
	}

	if (storageFlags & MoveStorageFlags::CaptureFlag) {
		U64 atkBB;
		if (flags != MoveFlags::EPCapture) atkBB = toBB;
		else atkBB = epAttackTargets[mPos.turn][mPos.enPassant];
		mPos.board.createColoredPiece(opponent, mCaptureStack.pop(), atkBB);
	}
}
