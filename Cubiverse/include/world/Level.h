#pragma once

#include "world/Chunk.h"

class Level {
private:
	struct Node {
		int lvl;
		int count;
		Node* nodes[8]; // represents Chunk* array if leaf node

		Node(int lvl) : lvl(lvl), count(0), nodes() {
		}

		Node* GetNode(const Vector3I& cp) {
			return nodes[GetIndex(cp)];
		}

		Chunk* GetChunk(const Vector3I& cp) {
			return reinterpret_cast<Chunk*>(nodes[GetIndex(cp)]);
		}

		void SetNode(const Vector3I& cp, Node* ptr) {
			nodes[GetIndex(cp)] = ptr;
		}

		void SetChunk(const Vector3I& cp, Chunk* ptr) {
			nodes[GetIndex(cp)] = reinterpret_cast<Node*>(ptr);
		}

		int GetIndex(const Vector3I& cp) {
			return	cp.x >> lvl & 1 | (cp.y >> lvl & 1) << 1 | (cp.z >> lvl & 1) << 2;
		}

		bool IsLeaf() {
			return lvl == 0;
		}

		void Shutdown() {
			if (IsLeaf()) {
				for (int i = 0; i < 8; i++) {
					if (nodes[i]) {
						reinterpret_cast<Chunk*>(nodes[i])->Shutdown();
					}
				}
			} else {
				for (int i = 0; i < 8; i++) {
					if (nodes[i]) {
						nodes[i]->Shutdown();
					}
				}
			}
			delete this;
		}
	};

public:
	Node* root;
	Vector3I offset; // keeps the root node exactly flush in the positive octant

	Level() : root(nullptr) {
	}

	int Dim() {
		return root ? Dim(root->lvl) : 0;
	}

	void Shutdown() {
		if (root) {
			root->Shutdown();
			root = nullptr;
		}
	}

	void InsertChunk(Chunk* c) {
		if (!root) {
			root = new Node(0);
			offset = -(c->cpos & ~1);
		}

		Vector3I cp = c->cpos + offset;
		Expand(cp);

		Node* node = root;
		while (!node->IsLeaf()) {
			Node* nextNode = node->GetNode(cp);
			if (!nextNode) {
				nextNode = new Node(node->lvl - 1);
				node->SetNode(cp, nextNode);
				node->count++;
			}
			node = nextNode;
		}
		node->SetChunk(cp, c);
		node->count++;
	}

	void RemoveChunk(Vector3I cp) {
		if (!root) return;
		cp += offset;
		Remove(root, cp);
		if (root->count == 0) {
			delete root;
			root = nullptr;
		} else {
			Contract();
		}
	}

	Chunk* GetChunk(Vector3I cp)const{
		if (!root || !InsideRootNode(cp)) return nullptr;
		cp += offset;
		Node* node = root;
		do {
			if (node->IsLeaf()) {
				return node->GetChunk(cp);
			}
		} while (node = node->GetNode(cp));
		return nullptr;
	}

private:
	void Expand(Vector3I& cp) {
		while (!InsideRootNode(cp)) {
			Vector3I cpo = cp.Offset(-(1 << root->lvl));
			cpo.x = cpo.x < 0;
			cpo.y = cpo.y < 0;
			cpo.z = cpo.z < 0;
			Node* newRoot = new Node(root->lvl + 1);
			newRoot->nodes[cpo.x | (cpo.y << 1) | (cpo.z << 2)] = root;
			newRoot->count = 1;
			cpo <<= newRoot->lvl;
			offset += cpo;
			cp += cpo;
			root = newRoot;
		}
	}

	void Contract() {
		while (root->count == 1 && !root->IsLeaf()) {
			for (int i = 0; i < 8; i++) {
				if (root->nodes[i]) {
					Node* newRoot = root->nodes[i];
					offset += Vector3I(i & 1, (i >> 1) & 1, (i >> 2) & 1) << root->lvl;
					delete root;
					root = newRoot;
					break;
				}
			}
		}
	}

	void Remove(Node* node, const Vector3I& cp) {
		if (!node->IsLeaf()) {
			Node* nextNode = node->GetNode(cp);
			if (nextNode) {
				Remove(nextNode, cp);
				if (nextNode->count == 0) {
					delete nextNode;
					node->SetNode(cp, nullptr);
					node->count--;
				}
			}
		} else if (node->GetChunk(cp)) {
			node->GetChunk(cp)->Shutdown();
			node->SetChunk(cp, nullptr);
			node->count--;
		}
	}

	bool InsideRootNode(const Vector3I& cp)const {
		return GetNodePositionContainingPoint(root->lvl, cp).IsZero();
	}

	static Vector3I GetNodePositionContainingPoint(int lvl, const Vector3I& cp) {
		return cp & ~(Dim(lvl) - 1);
	}

	static int Dim(int lvl) {
		return 2 << lvl;
	}
};