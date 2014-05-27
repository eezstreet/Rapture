#pragma once
#include <sys_shared.h>

template <typename T>
struct QTreeNode {
	T x, y, w, h;
	QTreeNode( T x1, T y1, T w1, T h1) :
		x(x1),
		y(y1),
		w(w1),
		h(h1) {}
	QTreeNode() {
		x = y = w = h = (T)0;
	}
};

template<typename C, typename U>
class QuadTree {
private:
	U											x, y, w, h;
	unsigned									depth, maxDepth;
	vector<C*>			nodes;

	QuadTree<C, U>*		NW;
	QuadTree<C, U>*		NE;
	QuadTree<C, U>*		SW;
	QuadTree<C, U>*		SE;
public:
	QuadTree(unsigned int x1, unsigned int y1, unsigned int w1, unsigned int h1, unsigned d1, unsigned m1) :
		x(x1),
		y(y1),
		w(w1),
		h(h1),
		depth(d1),
		maxDepth(m1) {
			if(depth == maxDepth) {
				return;
			}

			NW = new QuadTree<C, U>(x, y, w / (U)2, h / (U)2, depth+1, maxDepth);
			NE = new QuadTree<C, U>(x + w / (U)2, y, w / (U)2, h / (U)2, depth+1, maxDepth);
			SW = new QuadTree<C, U>(x, y + h / (U)2, w / (U)2, h / (U)2, depth+1, maxDepth);
			SE = new QuadTree<C, U>(x + w / (U)2, y + h / (U)2, w / (U)2, h / (U)2, depth+1, maxDepth);
	}

	~QuadTree() {
		if(depth == maxDepth) {
			return;
		}

		delete NW;
		delete NE;
		delete SW;
		delete SE;
	}

	bool Contains(QuadTree<C, U>* child, C* node) {
		return !(node->x < child->x ||
					node->y < child->y ||
					node->x > child->x + child->w ||
					node->y > child->y + child->h ||
					node->x + node->w < child->x ||
					node->y + node->h < child->y ||
					node->x + node->w > child->x + child->w ||
					node->y + node->h > child->y + child->h);
	}

	void AddNode(C* node) {
		if(depth == maxDepth) {
			nodes.push_back(node);
			return;
		}

		if(Contains(NW, node)) {
			NW->AddNode(node);
			return;
		} else if(Contains(NE, node)) {
			NE->AddNode(node);
			return;
		} else if(Contains(SW, node)) {
			SW->AddNode(node);
			return;
		} else if(Contains(SE, node)) {
			SE->AddNode(node);
			return;
		}
		if(Contains(this, node)) {
			nodes.push_back(node);
		}
	}

	vector<C*> NodesAt(const U posX, const U posY) {
		if(depth == maxDepth) {
			return nodes;
		}

		vector<C*> returnNodes, childReturnNodes;
		if(!nodes.empty()) {
			returnNodes = nodes;
		}
		// TODO: eliminate code reuse below
		if(posX > x + w / (U)2 && posY < y + h) {
			if(posY > y + h / (U)2 && posY < y + h) {
				childReturnNodes = SE->NodesAt(posX, posY);
				returnNodes.insert(returnNodes.end(), childReturnNodes.begin(), childReturnNodes.end());
				return returnNodes;
			} else if(posY > y && posY <= y + h / (U)2) {
				childReturnNodes = NE->NodesAt(posX, posY);
				returnNodes.insert(returnNodes.end(), childReturnNodes.begin(), childReturnNodes.end());
				return returnNodes;
			}
		} else if(posX > x && posX <= x + w / (U)2) {
			if(posY > y + h / (U)2 && posY < y + h) {
				childReturnNodes = SW->NodesAt(posX, posY);
				returnNodes.insert(returnNodes.end(), childReturnNodes.begin(), childReturnNodes.end());
				return returnNodes;
			} else if(posY > y && posY <= y + h / (U)2) {
				childReturnNodes = NW->NodesAt(posX, posY);
				returnNodes.insert(returnNodes.end(), childReturnNodes.begin(), childReturnNodes.end());
				return returnNodes;
			}
		}
		return returnNodes;
	}

	void Clear() {
		if(depth == maxDepth) {
			nodes.clear();
			return;
		} else {
			NW->Clear();
			NE->Clear();
			SW->Clear();
			SE->Clear();
		}
		
		if(!nodes.empty()) {
			nodes.clear();
		}
	}
};