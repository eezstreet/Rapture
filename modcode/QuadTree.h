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
	QuadTree<C, U>*		parent;
public:
	QuadTree(unsigned int x1, unsigned int y1, unsigned int w1, unsigned int h1, unsigned d1, unsigned m1, QuadTree<C, U>* ptParent) :
		x(x1),
		y(y1),
		w(w1),
		h(h1),
		depth(d1),
		maxDepth(m1),
		parent(ptParent) {
			if(depth == maxDepth) {
				return;
			}

			NW = new QuadTree<C, U>(x, y, w / (U)2, h / (U)2, depth+1, maxDepth, this);
			NE = new QuadTree<C, U>(x + w / (U)2, y, w / (U)2, h / (U)2, depth+1, maxDepth, this);
			SW = new QuadTree<C, U>(x, y + h / (U)2, w / (U)2, h / (U)2, depth+1, maxDepth, this);
			SE = new QuadTree<C, U>(x + w / (U)2, y + h / (U)2, w / (U)2, h / (U)2, depth+1, maxDepth, this);
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

	QuadTree<C, U>* AddNode(C* node) {
		if(depth == maxDepth) {
			nodes.push_back(node);
			return this;
		}

		if(Contains(NW, node)) {
			return NW->AddNode(node);
		} else if(Contains(NE, node)) {
			return NE->AddNode(node);
		} else if(Contains(SW, node)) {
			return SW->AddNode(node);
		} else if(Contains(SE, node)) {
			return SE->AddNode(node);
		}
		if(Contains(this, node)) {
			nodes.push_back(node);
		}

		return this;
	}

	void RemoveNode(C* node) {
		auto foundNode = find(nodes.begin(), nodes.end(), node);
		bool bDoesntContainNode = foundNode == nodes.end();
		if(bDoesntContainNode) {
			if(depth != maxDepth) {
				NW->RemoveNode(node);
				NE->RemoveNode(node);
				SW->RemoveNode(node);
				SE->RemoveNode(node);
			}
		}
		else {
			nodes.erase(foundNode);
		}
	}

	QuadTree<C, U>* ContainingTree(const U posX, const U posY) {
		if(depth == maxDepth) {
			return this;
		}

		if(posX > w + w / (U) 2 && posY < y + h) {
			if(posY > y + h / (U)2 && posY < y + h) {
				return SE->ContainingTree(posX, posY);
			} else if(posY > y && posY <= y + h / (U)2) {
				return NE->ContainingTree(posX, posY);
			}
		} else if(posX > x && posX <= x + w / (U)2) {
			if(posY > y + h / (U)2 && posY < y + h) {
				return SW->ContainingTree(posX, posY);
			} else if(posY > y && posY <= y + h / (U)2) {
				return NW->ContainingTree(posX, posY);
			}
		}

		return this;
	}
#if 0
	vector<C*> NodesIn(const U posX, const U posY, const U width, const U height) {
		if(depth == maxDepth) {
			return nodes;
		}

		vector<C*> returnNodes, childReturnNodes;
		if(!nodes.empty()) {
			returnNodes = nodes;
		}
		// TODO: eliminate code reuse below
		// TODO: finish
	}
#endif

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