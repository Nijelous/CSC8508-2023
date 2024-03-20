#pragma once
#include "Vector2.h"
#include "CollisionDetection.h"

namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		template<class T>
		class QuadTree;

		template<class T>
		struct QuadTreeEntry {
			Vector3 pos;
			Vector3 size;
			T object;

			QuadTreeEntry(T obj, Vector3 pos, Vector3 size) {
				object = obj;
				this->pos = pos;
				this->size = size;
			}
		};

		template<class T>
		class QuadTreeNode {
		public:
			typedef std::function<void(std::list<QuadTreeEntry<T>>&)> QuadTreeFunc;
		protected:
			friend class QuadTree<T>;

			QuadTreeNode() {}

			QuadTreeNode(Vector2 pos, Vector2 size) {
				children = nullptr;
				this->position = pos;
				this->size = size;
				isStatic = true;
			}

			~QuadTreeNode() {
				delete[] children;
			}

			void Insert(T& object, const Vector3& objectPos, const Vector3& objectSize, int depthLeft, int maxSize, bool addingStatic) {
				if (!CollisionDetection::AABBTest(objectPos, Vector3(position.x, 0, position.y), objectSize, Vector3(size.x, 1000.0f, size.y))) return;
				if (!addingStatic) isStatic = false;
				if (children) {
					for (int i = 0; i < 4; i++) {
						children[i].Insert(object, objectPos, objectSize, depthLeft - 1, maxSize, addingStatic);
					}
				}
				else {
					contents.push_back(QuadTreeEntry<T>(object, objectPos, objectSize));
					if ((int)contents.size() > maxSize && depthLeft > 0) {
						if (!children) {
							Split();
							for (const auto& i : contents) {
								for (int j = 0; j < 4; j++) {
									auto entry = i;
									children[j].Insert(entry.object, entry.pos, entry.size, depthLeft - 1, maxSize, addingStatic);
								}
							}
							contents.clear();
						}
					}
				}
			}

			void Split() {
				Vector2 halfSize = size / 2.0f;
				children = new QuadTreeNode<T>[4];
				children[0] = QuadTreeNode<T>(position + Vector2(-halfSize.x, halfSize.y), halfSize);
				children[1] = QuadTreeNode<T>(position + Vector2(halfSize.x, halfSize.y), halfSize);
				children[2] = QuadTreeNode<T>(position + Vector2(-halfSize.x, -halfSize.y), halfSize);
				children[3] = QuadTreeNode<T>(position + Vector2(halfSize.x, -halfSize.y), halfSize);
			}

			void DebugDraw() {
			}

			void CopyNode(QuadTreeNode<T>* node) {
				this->contents = node->contents;
				if (node->children) {
					Split();
					for (int i = 0; i < 4; i++) {
						this->children[i].CopyNode(&(node->children[i]));
					}
				}
			}

			void OperateOnContents(QuadTreeFunc& func) {
				if (children && !isStatic) {
					for (int i = 0; i < 4; i++) {
						children[i].OperateOnContents(func);
					}
				}
				else {
					if (!contents.empty()) {
						func(contents);
					}
				}
			}

			void OperateOnLeaf(QuadTreeFunc& func, const Vector3& objPos, const Vector3& objSize) {
				if (!CollisionDetection::AABBTest(objPos, Vector3(position.x, 0, position.y), objSize, Vector3(size.x, 1000.0f, size.y))) return;
				if (children) {
					for (int i = 0; i < 4; i++) {
						children[i].OperateOnLeaf(func, objPos, objSize);
					}
				}
				else {
					func(contents);
				}
			}

		protected:
			std::list< QuadTreeEntry<T> >	contents;

			Vector2 position;
			Vector2 size;

			bool isStatic;

			QuadTreeNode<T>* children;
		};
	}
}


namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		template<class T>
		class QuadTree {
		public:
			QuadTree() {}
			QuadTree(Vector2 size, int maxDepth = 6, int maxSize = 5) {
				root = QuadTreeNode<T>(Vector2(), size);
				this->maxDepth = maxDepth;
				this->maxSize = maxSize;
			}
			~QuadTree() {
			}

			void Insert(T object, const Vector3& pos, const Vector3& size, bool addingStatic) {
				root.Insert(object, pos, size, maxDepth, maxSize, addingStatic);
			}

			void DebugDraw() {
				root.DebugDraw();
			}

			void OperateOnContents(typename QuadTreeNode<T>::QuadTreeFunc  func) {
				root.OperateOnContents(func);
			}

			void OperateOnLeaf(typename QuadTreeNode<T>::QuadTreeFunc func, const Vector3& objPos, const Vector3& objSize) {
				root.OperateOnLeaf(func, objPos, objSize);
			}

			void CopyTree(QuadTree<T>* baseTree, Vector2 size) {
				root = QuadTreeNode<T>(Vector2(), size);
				root.CopyNode(&baseTree->root);
				this->maxDepth = baseTree->maxDepth;
				this->maxSize = baseTree->maxSize;
			}

			bool Empty() {
				return !root.children && root.contents.empty();
			}

		protected:
			QuadTreeNode<T> root;
			int maxDepth;
			int maxSize;
		};
	}
}