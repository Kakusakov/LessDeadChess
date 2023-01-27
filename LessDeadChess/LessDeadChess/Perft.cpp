#include "Perft.h"

MoveGen<1> MoveNode::mNodeExpander = { Position(classicStartPosFen) };

size_t MoveNode::countDepth() {
	size_t depth = 0;
	MoveNode* node = mParent;
	while (node != NULL) {
		depth += 1;
		node = node->mParent;
	}
	return depth;
}

void MoveNode::expand() {
	if (mNodeStatus != NodeStatus::Unexplored) return;
	mNodeExpander.resetRoot(mPos);
	switch (mNodeExpander.generateMoves()) {
	case GeneratedNodeResult::None:
		mNodeStatus = NodeStatus::None;
		break;
	case GeneratedNodeResult::Checkmate:
		mNodeStatus = NodeStatus::Checkmate;
		return;
	case GeneratedNodeResult::Stalemate:
		mNodeStatus = NodeStatus::Stalemate;
		return;
	case GeneratedNodeResult::Draw50Moves:
		mNodeStatus = NodeStatus::Draw50Moves;
		return;
	default:
		throw std::logic_error("found unknown value of generated move result");
	}
	std::vector<Position> childPositions = mNodeExpander.collectMovesAndFinish();
	mChildren.reserve(childPositions.size());
	for (const Position& pos : childPositions) {
		mChildren.push_back(MoveNode(pos, this));
	}
}

void handControllPerft(const Position& rootPos) {
	MoveNode root = { rootPos };
	MoveNode* pNode = &root;

	while (true) {
		std::string output = "\n\n\n\n";
		output += "depth=" + std::to_string(pNode->countDepth()) + "\n\n";
		output += "nodeStatus=";
		switch (pNode->getNodeStatus())
		{
		case NodeStatus::Unexplored: output += "Unexplored\n\n"; break;
		case NodeStatus::None: output += "None\n\n"; break;
		case NodeStatus::Checkmate: output += "Checkmate\n\n"; break;
		case NodeStatus::Stalemate: output += "Stalemate\n\n"; break;
		case NodeStatus::Draw50Moves: output += "Draw50Moves\n\n"; break;
		}
		output += "nodePosition=" + pNode->getPosition().toDebugAsciiView() + "\n";
		output += "childCount=" + std::to_string(pNode->getChildren().size()) + "\n";

		output += "\n";
		output += "controlls:\n"
			"\te => expandNode.\n"
			"\tc[n] => go to nth child.\n"
			"\tu => go up the parent tree.\n"
			"\tv => view child nodes.\n"
			"\tq => quit hand controll.\n";
		std::cout << output << std::endl;

		std::string input;
		std::cin >> input;
		if (input == "q") {
			std::cout << "quiting hand controll...\n";
			return;
		}
		else if (input == "u") {
			if (pNode->getParent() == NULL) {
				std::cout << "Command failed: this node has no parent!\n";
				continue;
			}
			pNode = pNode->getParent();
		}
		else if (input == "e") pNode->expand();
		else if (!input.empty() && input[0] == 'c') {
			std::string childIdxString = input.substr(1);
			if (std::find_if(
					childIdxString.begin(),
					childIdxString.end(),
					[](unsigned char c) { return !std::isdigit(c); }
				) != childIdxString.end()
			) {
				std::cout << "Command failed: the trailing string is not a number!\n";
				continue;
			}
			int childIdx = std::stoi(childIdxString);
			if (childIdx < 0 || childIdx >= pNode->getChildren().size()) {
				std::cout << "Command failed: the child index is out of range!\n";
				continue;
			}
			pNode = &(pNode->getChildren()[childIdx]);
		}
		else if (input == "v") {
			if (pNode->getChildren().empty()) {
				std::cout << "Command failed: the node has no children to view!\n";
				continue;
			}
			size_t childIdx = 0;
			while (true) {
				std::cout << "\n\n\n\n"
					+ std::to_string(childIdx + 1) + "/"
					+ std::to_string(pNode->getChildren().size()) + "\n\n"
					+ "Child=" + pNode->getChildren()[childIdx].getPosition().toDebugAsciiView()
					+ "\n"
					"Controlls:\n"
					"\tn => go to next child.\n"
					"\tp => go to previous child.\n"
					"\tq => quit child viewer.\n" << std::endl;
				std::string viewerInput;
				std::cin >> viewerInput;
				if (viewerInput == "n") {
					if (childIdx + 1 == pNode->getChildren().size()) {
						std::cout << "Command failed: can't go past the last child!\n";
						continue;
					}
					++childIdx;
				}
				else if (viewerInput == "p") {
					if (childIdx == 0) {
						std::cout << "Command failed: can't go before the first child!\n";
						continue;
					}
					--childIdx;
				}
				else if (viewerInput == "q") break;
				else std::cout << "Command failed: failed to recognize command!\n";
			}
		}
		else std::cout << "Command failed: failed to recognize command!\n";
	}
}
