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

#if 0
class QuadTreeI {
private:
	unsigned int								x, y, w, h;
	unsigned									depth, maxDepth;
	vector<QTreeNode<unsigned int>*>			nodes;

	QuadTreeI*		NW;
	QuadTreeI*		NE;
	QuadTreeI*		SW;
	QuadTreeI*		SE;
public:
	bool				Contains(QuadTreeI* child, QTreeNode<unsigned int>* node);

	QuadTreeI(unsigned int x1, unsigned int y1, unsigned int w1, unsigned int h1, unsigned d1, unsigned m1) :
		x(x1),
		y(y1),
		w(w1),
		h(h1),
		depth(d1),
		maxDepth(m1) {
			if(depth == maxDepth) {
				return;
			}

			NW = new QuadTreeI(x, y, w / 2, h / 2, depth+1, maxDepth);
			NE = new QuadTreeI(x + w / 2, y, w / 2, h / 2, depth+1, maxDepth);
			SW = new QuadTreeI(x, y + h / 2, w / 2, h / 2, depth+1, maxDepth);
			SE = new QuadTreeI(x + w / 2, y + h / 2, w / 2, h / 2, depth+1, maxDepth);
	}

	~QuadTreeI() {
		if(depth == maxDepth) {
			return;
		}

		delete NW;
		delete NE;
		delete SW;
		delete SE;
	}

	bool Contains(const QuadTreeI* child, const QTreeNode<unsigned int>* node) {
		return !(node->x < child->x ||
					node->y < child->y ||
					node->x > child->x + child->w ||
					node->y > child->y + child->h ||
					node->x + node->w < child->x ||
					node->y + node->h < child->y ||
					node->x + node->w > child->x + child->w ||
					node->y + node->h > child->y + child->h);
	}

	void AddNode(QTreeNode<unsigned int>* node) {
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

	vector<QTreeNode<unsigned int>*> NodesAt(const unsigned int posX, const unsigned int posY) {
		if(depth == maxDepth) {
			return nodes;
		}

		vector<QTreeNode<unsigned int>*> returnNodes, childReturnNodes;
		if(!nodes.empty()) {
			returnNodes = nodes;
		}
		// TODO: eliminate code reuse below
		if(posX > x + w / 2 && posY < y + h) {
			if(posY > y + h / 2 && posY < y + h) {
				childReturnNodes = SE->NodesAt(posX, posY);
				returnNodes.insert(returnNodes.end(), childReturnNodes.begin(), childReturnNodes.end());
				return returnNodes;
			} else if(posY > y && posY <= y + h / 2) {
				childReturnNodes = NE->NodesAt(posX, posY);
				returnNodes.insert(returnNodes.end(), childReturnNodes.begin(), childReturnNodes.end());
				return returnNodes;
			}
		} else if(posX > x && posX <= x + w / 2) {
			if(posY > y + h / 2 && posY < y + h) {
				childReturnNodes = SW->NodesAt(posX, posY);
				returnNodes.insert(returnNodes.end(), childReturnNodes.begin(), childReturnNodes.end());
				return returnNodes;
			} else if(posY > y && posY <= y + h / 2) {
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
#endif

// X = class name
// Y = node class
// Z = scale type (unsigned int/float)
#define QuadTree(X, Y, Z)																			\
class X {																							\
private:																							\
	Z				x, y, w, h;																		\
	unsigned		depth, maxDepth;																\
	vector<Y*>		nodes;																			\
																									\
	X*				NW;																				\
	X*				NE;																				\
	X*				SW;																				\
	X*				SE;																				\
public:																								\
	X(Z x1, Z y1, Z w1, Z h1, unsigned d1, unsigned m1) :											\
		x(x1),																						\
		y(y1),																						\
		w(w1),																						\
		h(h1),																						\
		depth(d1),																					\
		maxDepth(m1) {																				\
			if(depth == maxDepth) {																	\
				return;																				\
			}																						\
																									\
			NW = new X(x, y, w / (Z)2, h / (Z)2, depth+1, maxDepth);								\
			NE = new X(x + w / (Z)2, y, w / (Z)2, h / (Z)2, depth+1, maxDepth);						\
			SW = new X(x, y + h / (Z)2, w / (Z)2, h / (Z)2, depth+1, maxDepth);						\
			SE = new X(x + w / (Z)2, y + h / (Z)2, w / (Z)2, h / (Z)2, depth+1, maxDepth);			\
	}																								\
																									\
	~X() {																							\
		if(depth == maxDepth) {																		\
			return;																					\
		}																							\
																									\
		delete NW;																					\
		delete NE;																					\
		delete SW;																					\
		delete SE;																					\
	}																								\
																									\
	bool Contains(X* child, Y* node) {																\
		return !(node->x < child->x ||																\
					node->y < child->y ||															\
					node->x > child->x + child->w ||												\
					node->y > child->y + child->h ||												\
					node->x + node->w < child->x ||													\
					node->y + node->h < child->y ||													\
					node->x + node->w > child->x + child->w ||										\
					node->y + node->h > child->y + child->h);										\
	}																								\
																									\
	void AddNode(Y* node) {																			\
		if(depth == maxDepth) {																		\
			nodes.push_back(node);																	\
			return;																					\
		}																							\
																									\
		if(Contains(NW, node)) {																	\
			NW->AddNode(node);																		\
			return;																					\
		} else if(Contains(NE, node)) {																\
			NE->AddNode(node);																		\
			return;																					\
		} else if(Contains(SW, node)) {																\
			SW->AddNode(node);																		\
			return;																					\
		} else if(Contains(SE, node)) {																\
			SE->AddNode(node);																		\
			return;																					\
		}																							\
		if(Contains(this, node)) {																	\
			nodes.push_back(node);																	\
		}																							\
	}																								\
																									\
	vector<Y*> NodesAt(const Z posX, const Z posY) {												\
		if(depth == maxDepth) {																		\
			return nodes;																			\
		}																							\
																									\
		vector<Y*> returnNodes, childReturnNodes;													\
		if(!nodes.empty()) {																		\
			returnNodes = nodes;																	\
		}																							\
		if(posX > x + w / (Z)2 && posY < y + h) {													\
			if(posY > y + h / (Z)2 && posY < y + h) {												\
				childReturnNodes = SE->NodesAt(posX, posY);											\
				returnNodes.insert(returnNodes.end(), childReturnNodes.begin(), childReturnNodes.end()); \
				return returnNodes;																	\
			} else if(posY > y && posY <= y + h / (Z)2) {											\
				childReturnNodes = NE->NodesAt(posX, posY);											\
				returnNodes.insert(returnNodes.end(), childReturnNodes.begin(), childReturnNodes.end()); \
				return returnNodes;																	\
			}																						\
		} else if(posX > x && posX <= x + w / (Z)2) {												\
			if(posY > y + h / 2 && posY < y + h) {													\
				childReturnNodes = SW->NodesAt(posX, posY);											\
				returnNodes.insert(returnNodes.end(), childReturnNodes.begin(), childReturnNodes.end()); \
				return returnNodes;																	\
			} else if(posY > y && posY <= y + h / (Z)2) {											\
				childReturnNodes = NW->NodesAt(posX, posY);											\
				returnNodes.insert(returnNodes.end(), childReturnNodes.begin(), childReturnNodes.end()); \
				return returnNodes;																	\
			}																						\
		}																							\
		return returnNodes;																			\
	}																								\
																									\
	void Clear() {																					\
		if(depth == maxDepth) {																		\
			nodes.clear();																			\
			return;																					\
		} else {																					\
			NW->Clear();																			\
			NE->Clear();																			\
			SW->Clear();																			\
			SE->Clear();																			\
		}																							\
																									\
		if(!nodes.empty()) {																		\
			nodes.clear();																			\
		}																							\
	}																								\
};																									