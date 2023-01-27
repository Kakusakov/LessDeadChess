#include "Position.h"
#include <stdexcept>
#include <sstream>
#include <iterator>
#include <cctype>

std::string Position::toDebugAsciiView() const {
	std::string ret = "Positon[turn=" + std::to_string(turn)
		+ ", cRights=" + std::to_string(cRights)
		+ ", enPassant=" + std::to_string(enPassant)
		+ ", hmClock=" + std::to_string(hmClock)
		+ "]\n{\n";
	//std::cout << "whites: ";
	auto whites = serializeBB(board.getBB(Board::PieceBB::White));
	/*for (const auto& sq : whites) {
		std::cout << std::to_string(sq) + " ";
	}
	std::cout << "\n" + BBToString(board.getBB(Board::PieceBB::White)) + "\n";
	std::cout << "blacks: ";
	auto balcks = serializeBB(board.getBB(Board::PieceBB::Black));
	for (const auto& sq : balcks) {
		std::cout << std::to_string(sq) + " ";
	}
	std::cout << "\n" + BBToString(board.getBB(Board::PieceBB::Black)) + "\n";
	std::cout << "pawns: ";*/
	auto pawns = serializeBB(board.getBB(Board::PieceBB::Pawn));
	/*for (const auto& sq : pawns) {
		std::cout << std::to_string(sq) + " ";
	}
	std::cout << "\n" + BBToString(board.getBB(Board::PieceBB::Pawn)) + "\n";
	std::cout << "knights: ";*/
	auto knights = serializeBB(board.getBB(Board::PieceBB::Knight));
	/*for (const auto& sq : knights) {
		std::cout << std::to_string(sq) + " ";
	}
	std::cout << "\n" + BBToString(board.getBB(Board::PieceBB::Knight)) + "\n";*/
	auto bishops = serializeBB(board.getBB(Board::PieceBB::Bishop));
	auto rooks = serializeBB(board.getBB(Board::PieceBB::Rook));
	auto queens = serializeBB(board.getBB(Board::PieceBB::Queen));
	auto kings = serializeBB(board.getBB(Board::PieceBB::King));
	std::string viev = "";
	std::string row;
	for (int y = 7; y >= 0; --y) {
		row = "";
		for (int x = 7; x >= 0; --x) {
			Square sq = (Square)(y * 8 + x);
			bool found = true;
			if (std::find(pawns.begin(), pawns.end(), sq) != pawns.end()) {
				row = "P" + row;
			}
			else if (std::find(knights.begin(), knights.end(), sq) != knights.end()) {
				row = "N" + row;
			}
			else if (std::find(bishops.begin(), bishops.end(), sq) != bishops.end()) {
				row = "B" + row;
			}
			else if (std::find(rooks.begin(), rooks.end(), sq) != rooks.end()) {
				row = "R" + row;
			}
			else if (std::find(queens.begin(), queens.end(), sq) != queens.end()) {
				row = "Q" + row;
			}
			else if (std::find(kings.begin(), kings.end(), sq) != kings.end()) {
				row = "K" + row;
			}
			else {
				found = false;
				row = "__" + row;
			}

			if (found) {
				if (std::find(whites.begin(), whites.end(), sq) != whites.end()) row = "W" + row;
				else row = "B" + row;
			}
			row = " " + row;
		}
		viev += row + "\n";
	}
	ret += viev + "}\n";
	return ret;
}

Position::Position(std::string fen) {
	std::istringstream iss(fen);
	std::vector<std::string> fenTokens{
		std::istream_iterator<std::string>{iss},
		std::istream_iterator<std::string>{}
	};
	if (fenTokens.size() != 6) {
		throw std::logic_error("Bad FEN size=" + std::to_string(fenTokens.size()));
	}
	const std::string postionStr = fenTokens[0];
	const std::string& turnStr = fenTokens[1];
	const std::string& ÒRightsStr = fenTokens[2];
	const std::string& enPassantStr = fenTokens[3];
	const std::string& hmClockStr = fenTokens[4];
	const std::string& fmClockStr = fenTokens[5];

	// parse Position
	std::stringstream positionSS(postionStr);
	std::string segment;
	std::vector<std::string> positionTokens;

	while (std::getline(positionSS, segment, '/'))
	{
		positionTokens.push_back(segment);
	}
	if (positionTokens.size() != 8) {
		throw std::logic_error("Bad position_rows_count=" + std::to_string(positionTokens.size()));
	}
	U64 sqBB = C64(1);
	this->board = {};
	for (int y = 7; y >= 0; --y) {
		size_t extraX = 0;
		for (size_t x = 0; x < positionTokens[y].size(); ++x, sqBB <<= 1) {
			char c = positionTokens[y][x];
			if (c >= '1' && c <= '8') {
				extraX += c - '1';
				sqBB <<= c - '1';
				if (x + extraX > 8) {
					throw std::logic_error("FEN row skips to many chars=" + std::to_string(x + extraX));
				}
				continue;
			}
			char lower = tolower(c);
			Color color = islower(c) ? Color::Black : Color::White;
			if (lower == 'k') this->board.createColoredPieces(color, Piece::King, sqBB);
			else if (lower == 'q') this->board.createColoredPieces(color, Piece::Queen, sqBB);
			else if (lower == 'r') this->board.createColoredPieces(color, Piece::Rook, sqBB);
			else if (lower == 'b') this->board.createColoredPieces(color, Piece::Bishop, sqBB);
			else if (lower == 'n') this->board.createColoredPieces(color, Piece::Knight, sqBB);
			else if (lower == 'p') this->board.createColoredPieces(color, Piece::Pawn, sqBB);
			else throw std::logic_error("unknown FEN position char=" + c);
		}
	}

	// parse turn
	if (turnStr == "w") this->turn = Color::White;
	else if (turnStr == "b") this->turn = Color::Black;
	else throw std::logic_error("Bad FEN turn=" + turnStr);

	// parse ÒRights
	this->cRights = (CRightsFlags)0;
	if (ÒRightsStr != "-") {
		int rightsSize = ÒRightsStr.size();
		if (ÒRightsStr.find("K") != std::string::npos) {
			--rightsSize;
			this->cRights = (CRightsFlags)(this->cRights | CRightsFlags::WhiteKing);
		}
		if (ÒRightsStr.find("Q") != std::string::npos) {
			--rightsSize;
			this->cRights = (CRightsFlags)(this->cRights | CRightsFlags::WhiteQueen);
		}
		if (ÒRightsStr.find("k") != std::string::npos) {
			--rightsSize;
			this->cRights = (CRightsFlags)(this->cRights | CRightsFlags::BlackKing);
		}
		if (ÒRightsStr.find("q") != std::string::npos) {
			--rightsSize;
			this->cRights = (CRightsFlags)(this->cRights | CRightsFlags::BlackQueen);
		}
		if (rightsSize != 0) {
			throw std::logic_error("castling_rights=" + ÒRightsStr
				+ " contain " + std::to_string(rightsSize) + " unexpected characters"
			);
		}
	}

	// parse enPassant
	if (enPassantStr != "-") {
		if (enPassantStr.size() != 2) {
			throw std::logic_error("Bad en_passant_size=" + std::to_string(enPassantStr.size()));
		}
		int enPassantCol = enPassantStr[0] - 'a';
		int enPassantRow = enPassantStr[1] - '0';
		if (enPassantCol < 0
			|| enPassantCol > 7
			|| !(enPassantRow == 6 || enPassantRow == 3)
			) {
			throw std::logic_error("Bad en_passant" + enPassantStr);
		}
		if ((this->turn) == (enPassantRow == 6)) {
			throw std::logic_error("En passant is incompatible with turn, en_passant=" + enPassantStr
				+ " turn=" + turnStr
			);
		}
		this->enPassant = (EnPassant)enPassantCol;
	}
	else this->enPassant = EnPassant::None;

	int halfMoves = std::stoi(hmClockStr);
	if (halfMoves < 0) throw std::logic_error("Too small half_move_clock=" + hmClockStr);
	this->hmClock = halfMoves;
	// parse fm clock
	int fullMoves = std::stoi(fmClockStr);
	if (fullMoves < 1) throw std::logic_error("Too small full_move_clock=" + fmClockStr);
	
}

