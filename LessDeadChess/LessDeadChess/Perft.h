#pragma once
#include "MoveGen.h"

enum class NodeStatus {
	Unexplored,
	None,
	Checkmate,
	Stalemate,
	Draw50Moves,
};

class MoveNode
{
private:
	static MoveGen<1> mNodeExpander;
	MoveNode* mParent = NULL;
	Position mPos = { classicStartPosFen };
	std::vector<MoveNode> mChildren = {};
	NodeStatus mNodeStatus = NodeStatus::Unexplored;
public:
	MoveNode(Position pos) {
		mPos = pos;
	}
	MoveNode(Position pos, MoveNode* parent) {
		mPos = pos;
		mParent = parent;
	}
	inline MoveNode* getParent() const { return mParent; }
	inline const Position& getPosition() const { return mPos; }
	inline const NodeStatus& getNodeStatus() const { return mNodeStatus; }
	inline std::vector<MoveNode>& getChildren() { return mChildren; }
	size_t countDepth();
	void expand();
};

void handControllPerft(const Position& rootPos);
