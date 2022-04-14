#ifndef __WASTELADNS_BVH_H__
#define __WASTELADNS_BVH_H__

#include <vector>
#include <queue>
#include <functional>

namespace BVH {

	struct Node {
		Vec3 min;
		Vec3 max;
		u32 lchildId;
		u32 triangleId;
		bool isLeaf;
	};
	
	struct Triangle {
		Vec3 min;
		Vec3 max;
		Vec3 center;
		u32 firstVertexIndex;
	};
	struct BuildTreeContext {
		std::vector<Triangle> trianglePool;
		const f32* vertexPool;
		const u16* indexPool;
		u32 indexCount;
	};
	struct Tree {
		std::vector<Node> nodes;
	};
	void emptyNode(Node& n) {
		n.min = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		n.max = Vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	}
	void expandNodeBounds(Node& n, const Triangle& tri) {
		n.min = Math::min(n.min, tri.min);
		n.max = Math::max(n.max, tri.max);
	}
	void buildTreeRec(Tree& bvh, const u32 nodeId, const BuildTreeContext& context, const std::vector<u32>& triangleIds) {
		bvh.nodes[nodeId].isLeaf = triangleIds.size() == 1;
		if (bvh.nodes[nodeId].isLeaf) {
			const Triangle& tri = context.trianglePool[triangleIds[0]];
			bvh.nodes[nodeId].min = tri.min;
			bvh.nodes[nodeId].max = tri.max;
			bvh.nodes[nodeId].triangleId = triangleIds[0];
		}
		else {
			bvh.nodes[nodeId].lchildId = bvh.nodes.size();

			std::vector<u32> ltriangleIds;
			std::vector<u32> rtriangleIds;
			// Start off with the same size as our parent, to prevent resizing mid-way
			ltriangleIds.reserve(triangleIds.size());
			rtriangleIds.reserve(triangleIds.size());
			Node lchild;
			Node rchild;
			emptyNode(lchild);
			emptyNode(rchild);

			Vec3 currExtents = Math::subtract(bvh.nodes[nodeId].max, bvh.nodes[nodeId].min);
			u8 widestCoord = 0;
			if (currExtents.y > Math::max(currExtents.x, currExtents.z)) {
				widestCoord = 1;
			} else if (currExtents.z > Math::max(currExtents.x, currExtents.y)) {
				widestCoord = 2;
			}
			f32 widestCoordCenter = (bvh.nodes[nodeId].max.coords[widestCoord] + bvh.nodes[nodeId].min.coords[widestCoord]) * 0.5f;

			// Split triangles on each side of the widest axis
			for (u32 triangleId : triangleIds) {
				const Triangle& tri = context.trianglePool[triangleId];
				Node* selectedNode;
				std::vector <u32>* selectedTriangleIds;
				if (tri.center.coords[widestCoord] < widestCoordCenter) {
					selectedNode = &lchild;
					selectedTriangleIds = &ltriangleIds;
				} else {
					selectedNode = &rchild;
					selectedTriangleIds = &rtriangleIds;
				}

				expandNodeBounds(*selectedNode, tri);
				selectedTriangleIds->push_back(triangleId);
			}

			// One of the sides is empty (can happen, since the widest axis is determined from bounding boxes,
			// but the center of every triangle may lie on one side
			if (ltriangleIds.size() == 0 || rtriangleIds.size() == 0) {
				emptyNode(lchild);
				emptyNode(rchild);
				ltriangleIds.clear();
				rtriangleIds.clear();
				for (u32 i = 0; i < triangleIds.size(); i++) {
					const Triangle& tri = context.trianglePool[triangleIds[i]];
					Node* selectedNode;
					std::vector <u32>* selectedTriangleIds;
					// Pick one side for each triangle
					if ((i & 1) == 0) {
						selectedNode = &lchild;
						selectedTriangleIds = &ltriangleIds;
					}
					else {
						selectedNode = &rchild;
						selectedTriangleIds = &rtriangleIds;
					}
					expandNodeBounds(*selectedNode, tri);
					selectedTriangleIds->push_back(triangleIds[i]);
				}
			}

			// Note that it's not useful to keep a reference bvh.nodes[nodeId] inside this function,
			// since the array may move the location of the node during this reallocate-emplace
			bvh.nodes.push_back(lchild);
			bvh.nodes.push_back(rchild);
			buildTreeRec(bvh, bvh.nodes[nodeId].lchildId, context, ltriangleIds);
			buildTreeRec(bvh, bvh.nodes[nodeId].lchildId + 1, context, rtriangleIds);

			//bvh.nodes[nodeId].min = Math::min(lchild.min, rchild.min);
			//bvh.nodes[nodeId].max = Math::max(lchild.max, rchild.max);
		}
	}
	void buildTree(Tree& bvh, f32* vertexPool, const u16* indexPool, const u32 indexCount) {

		BuildTreeContext context;
		context.vertexPool = vertexPool;
		context.indexPool = indexPool;
		context.indexCount = indexCount;
		context.trianglePool.clear();
		context.trianglePool.reserve(context.indexCount / 3);

		std::vector<u32> triangleIds;
		u32 triangleCount = context.indexCount / 3;
		bvh.nodes.clear();
		Node root;
		emptyNode(root);

		for (u32 triangleId = 0; triangleId < triangleCount; triangleId++) {

			Triangle tri;

			Vec3 a(context.vertexPool[context.indexPool[triangleId * 3] * 3]
				 , context.vertexPool[context.indexPool[triangleId * 3] * 3 + 1]
				 , context.vertexPool[context.indexPool[triangleId * 3] * 3 + 2]);
			Vec3 b(context.vertexPool[context.indexPool[triangleId * 3 + 1] * 3]
				 , context.vertexPool[context.indexPool[triangleId * 3 + 1] * 3 + 1]
				 , context.vertexPool[context.indexPool[triangleId * 3 + 1] * 3 + 2]);
			Vec3 c(context.vertexPool[context.indexPool[triangleId * 3 + 2] * 3]
				 , context.vertexPool[context.indexPool[triangleId * 3 + 2] * 3 + 1]
				 , context.vertexPool[context.indexPool[triangleId * 3 + 2] * 3 + 2]);
			tri.min = Math::min(Math::min(a, b), c);
			tri.max = Math::max(Math::max(a, b), c);
			tri.center = Math::scale(Math::add(tri.max, tri.min), 0.5f);
			expandNodeBounds(root, tri);

			context.trianglePool.push_back(tri);
			triangleIds.push_back(triangleId);
		}

		bvh.nodes.push_back(root);
		buildTreeRec(bvh, 0, context, triangleIds);
	}

	void closestPointOnTriangleSq(Vec3& closestPoint, f32& closestDistSq, const Vec3& p, const Vec3& a, const Vec3& b, const Vec3& c) {
		Vec3 ba = Math::subtract(b, a);
		Vec3 pa = Math::subtract(p, a);
		Vec3 cb = Math::subtract(c, b);
		Vec3 pb = Math::subtract(p, b);
		Vec3 ac = Math::subtract(a, c);
		Vec3 pc = Math::subtract(p, c);
		Vec3 n = Math::cross(ba, ac);

		// check whether p lies to the right of the half space for each edge
		bool isPRightOfBA = Math::dot(Math::cross(ba, n), pa) > 0.f;
		bool isPRightOfCB = Math::dot(Math::cross(cb, n), pb) > 0.f;
		bool isPRightOfAC = Math::dot(Math::cross(ac, n), pc) > 0.f;
		bool insidePrism = isPRightOfBA && isPRightOfCB && isPRightOfAC;
		if (insidePrism) {
			// find closest point on plane
			// distance = dot(pa, n) / mag(n) -> distanceSq = dot(pa, n)^2 / dot(n, n)
			f32 pan = Math::dot(pa, n);
			f32 distanceSq = pan * pan / Math::dot(n, n);
			// desired p' = p - n * distance / mag(n) = p - n * dot(pa, n) / mag(n)^2 = p - n * distanceSq / dot(pa, n);
			closestPoint = Math::subtract(p, Math::scale(n, distanceSq / pan));
			closestDistSq = distanceSq;
		} else {
			// find closest point p', which is a distance d along closest edge ba, clamped to the edges of ba
			// edge a: p'p = p'a - pa = d * ba / mag(ba) - pa = clamp(dot(ba, pa) / mag(ba), 0, 1) * ba / mag(ba) - pa = (clamp(dot(ba, pa) / dot(ba, ba), 0, 1) * ba - pa
			Vec3 pp_ba = Math::subtract(Math::scale(ba, Math::clamp(Math::dot(ba, pa) / Math::dot(ba, ba), 0.f, 1.f)), pa);
			f32 distanceSqBA = Math::dot(pp_ba, pp_ba);
			Vec3 pp_cb = Math::subtract(Math::scale(cb, Math::clamp(Math::dot(cb, pb) / Math::dot(cb, cb), 0.f, 1.f)), pb);
			f32 distanceSqCB = Math::dot(pp_cb, pp_cb);
			Vec3 pp_ac = Math::subtract(Math::scale(ac, Math::clamp(Math::dot(ac, pc) / Math::dot(ac, ac), 0.f, 1.f)), pc);
			f32 distanceSqAC = Math::dot(pp_ac, pp_ac);
			if (distanceSqBA < distanceSqCB && distanceSqBA < distanceSqAC) {
				closestPoint = Math::add(pp_ba, p);
				closestDistSq = distanceSqBA;
			} else if (distanceSqCB < distanceSqBA && distanceSqCB < distanceSqAC) {
				closestPoint = Math::add(pp_cb, p);
				closestDistSq = distanceSqCB;
			} else {
				closestPoint = Math::add(pp_ac, p);
				closestDistSq = distanceSqAC;
			}
		}
	}
	f32 distanceToNodeSq(const Vec3& p, const Node& n) {
		Vec3 clamped = Math::max(Math::min(n.max, p), n.min);
		return Math::magSq(Math::subtract(clamped, p));
	}
	struct QueryNode {
		u32 nodeId;
		// distance to the node's bb (the minimum possible distance from the query point to a primitive inside the node)
		f32 distanceSq;
	};
	bool findClosestPoint(Vec3& resultPoint, const Tree& bvh, const Vec3 p, const bool filterOutsidePoints, f32* vertexPool, const u16* indexPool, const u32 indexCount) {
		// heap is sorted keeping the node with the shortest candidate distance on top
		auto cmp = [](const QueryNode& a, const QueryNode& b) { return a.distanceSq > b.distanceSq; };
		std::priority_queue<QueryNode, std::vector<QueryNode>, decltype(cmp)> nodeHeap(cmp);
		nodeHeap.push(QueryNode{ 0, 0.f });
		f32 closestDistanceSq = FLT_MAX;
		Vec3 closestPoint = {};
		u32 closestTriangleId = 0;

		// traverse the heap until the node with the shortest minimum distance possible can't get any better than the current candidate distance
		while (nodeHeap.size() > 0 && nodeHeap.top().distanceSq < closestDistanceSq) {
			QueryNode n = nodeHeap.top();
			nodeHeap.pop();

			// compute actual distance to leaf
			if (bvh.nodes[n.nodeId].isLeaf) {
				u32 triangleId = bvh.nodes[n.nodeId].triangleId;
				Vec3 a(vertexPool[indexPool[triangleId * 3] * 3]
					, vertexPool[indexPool[triangleId * 3] * 3 + 1]
					, vertexPool[indexPool[triangleId * 3] * 3 + 2]);
				Vec3 b(vertexPool[indexPool[triangleId * 3 + 1] * 3]
					, vertexPool[indexPool[triangleId * 3 + 1] * 3 + 1]
					, vertexPool[indexPool[triangleId * 3 + 1] * 3 + 2]);
				Vec3 c(vertexPool[indexPool[triangleId * 3 + 2] * 3]
					, vertexPool[indexPool[triangleId * 3 + 2] * 3 + 1]
					, vertexPool[indexPool[triangleId * 3 + 2] * 3 + 2]);

				Vec3 candidateClosestPoint;
				f32 candidateClosestDistanceSq;
				closestPointOnTriangleSq(candidateClosestPoint, candidateClosestDistanceSq, p, a, b, c);
				if (candidateClosestDistanceSq < closestDistanceSq) {
					closestDistanceSq = candidateClosestDistanceSq;
					closestPoint = candidateClosestPoint;
					closestTriangleId = triangleId;
				}
			}
			else {
				const Node& lchild = bvh.nodes[bvh.nodes[n.nodeId].lchildId];
				const Node& rchild = bvh.nodes[bvh.nodes[n.nodeId].lchildId + 1];
				const f32 ldistSq = distanceToNodeSq(p, lchild);
				const f32 rdistSq = distanceToNodeSq(p, rchild);

				// consider each child as long as their minimum possible distance is smaller than our candidate
				if (ldistSq < closestDistanceSq) {
					nodeHeap.push(QueryNode{ bvh.nodes[n.nodeId].lchildId, ldistSq });
				}
				if (rdistSq < closestDistanceSq) {
					nodeHeap.push(QueryNode{ bvh.nodes[n.nodeId].lchildId + 1, rdistSq });
				}
			}
		}

		resultPoint = closestPoint;
		bool result = true;
		if (filterOutsidePoints) {
			bool inside = false;
			{
				Vec3 a(vertexPool[indexPool[closestTriangleId * 3] * 3]
					, vertexPool[indexPool[closestTriangleId * 3] * 3 + 1]
					, vertexPool[indexPool[closestTriangleId * 3] * 3 + 2]);
				Vec3 b(vertexPool[indexPool[closestTriangleId * 3 + 1] * 3]
					, vertexPool[indexPool[closestTriangleId * 3 + 1] * 3 + 1]
					, vertexPool[indexPool[closestTriangleId * 3 + 1] * 3 + 2]);
				Vec3 c(vertexPool[indexPool[closestTriangleId * 3 + 2] * 3]
					, vertexPool[indexPool[closestTriangleId * 3 + 2] * 3 + 1]
					, vertexPool[indexPool[closestTriangleId * 3 + 2] * 3 + 2]);
				Vec3 n = Math::cross(Math::subtract(b, a), Math::subtract(c, a));

				const f32 g = Math::dot(Math::subtract(p, closestPoint), n);
				inside = g < 0.f;
			}
			result = inside;
		}

		return result;
	}

}; // namespace BVH

#endif // __WASTELADNS_BVH_H__
