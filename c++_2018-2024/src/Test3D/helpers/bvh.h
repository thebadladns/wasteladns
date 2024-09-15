#ifndef __WASTELADNS_BVH_H__
#define __WASTELADNS_BVH_H__

#include <vector>
#include <queue>
#include <float.h>

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
		tinystl::vector<Triangle> trianglePool;
		const f32* vertexPool;
		const u16* indexPool;
		u32 indexCount;
	};
	struct Tree {
		tinystl::vector<Node> nodes;
	};
	void emptyNode(Node& n) {
		n.min = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		n.max = Vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	}
	void expandNodeBounds(Node& n, const Triangle& tri) {
		n.min = Math::min(n.min, tri.min);
		n.max = Math::max(n.max, tri.max);
	}
	void buildTreeRec(Tree& bvh, const u32 nodeId, const BuildTreeContext& context, const tinystl::vector<u32>& triangleIds) {
		bvh.nodes[nodeId].isLeaf = triangleIds.size() == 1;
		if (bvh.nodes[nodeId].isLeaf) {
			const Triangle& tri = context.trianglePool[triangleIds[0]];
			bvh.nodes[nodeId].min = tri.min;
			bvh.nodes[nodeId].max = tri.max;
			bvh.nodes[nodeId].triangleId = triangleIds[0];
		}
		else {
			bvh.nodes[nodeId].lchildId = (u32)bvh.nodes.size();

			tinystl::vector<u32> ltriangleIds;
			tinystl::vector<u32> rtriangleIds;
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
				tinystl::vector <u32>* selectedTriangleIds;
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
					tinystl::vector <u32>* selectedTriangleIds;
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

		tinystl::vector<u32> triangleIds;
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

	bool queryRayIntersectsWithTriangle(f32& t, const Vec3& p, const Vec3& dir, const Vec3& a, const Vec3& b, const Vec3& c) {
		// using barycentric coordinates ( see https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection )
		Vec3 ba = Math::subtract(b, a);
		Vec3 ca = Math::subtract(c, a);
		Vec3 pa = Math::subtract(p, a);
		Vec3 n = Math::cross(ba, ca);
		Vec3 q = Math::cross(pa, dir);
		f32 d = 1.f / Math::dot(dir, n);
		f32 u = d * Math::dot(Math::negate(q), ca);
		f32 v = d * Math::dot(q, ba);
		t = d * Math::dot(Math::negate(n), pa);
		if (u < 0.f || v < 0.f || (u + v) > 1.f) t = -1.f;
		return t > 0.f;
	}
	bool queryRayIntersectsWithBox(f32& t, const Vec3& p, const Vec3& dir, const Vec3& min, const Vec3& max) {

		// slab method: for each axis, find the minT and maxT of the ray across the two sides of the box
		// by triangle proportionality, and because dir is normalized, minx-px/minTx = dirx/1, so minTx = minx-px/dirx;
		Vec3 dirfrac;
		dirfrac.x = 1.0f / dir.x;
		dirfrac.y = 1.0f / dir.y;
		dirfrac.z = 1.0f / dir.z;
		float t1 = (min.x - p.x) * dirfrac.x;
		float t2 = (max.x - p.x) * dirfrac.x;
		float t3 = (min.y - p.y) * dirfrac.y;
		float t4 = (max.y - p.y) * dirfrac.y;
		float t5 = (min.z - p.z) * dirfrac.z;
		float t6 = (max.z - p.z) * dirfrac.z;

		// combine all intersections together
		float minT = Math::max( Math::max( Math::min(t1, t2),  Math::min(t3, t4)),  Math::min(t5, t6));
		float maxT = Math::min( Math::min( Math::max(t1, t2),  Math::max(t3, t4)),  Math::max(t5, t6));

		// neg maxT: the ray starts past the box
		if (maxT < 0.f) {
			t = maxT;
			return false;
		}
		// minT > maxT: the ray doesn't intersect
		if (minT > maxT)
		{
			t = maxT;
			return false;
		}
		// minT < 0: the ray starts inside the box, so maxT is the next hit
		if (minT < 0) {
			t = maxT;
		} else {
			t = minT;
		}
		return true;
	}
	struct InsideQueryNode {
		u32 nodeId;
	};
	bool queryIsPointInside(tinystl::vector<f32>& closestTs, const Tree& bvh, const Vec3& p, const Vec3& biasDir, f32* vertexPool, const u16* indexPool, const u32 indexCount) {
		std::queue<InsideQueryNode> nodeQueue;
		nodeQueue.push(InsideQueryNode{ 0 });
		u32 intersections = 0;
		Vec3 dir = biasDir;
		while (nodeQueue.size() > 0) {
			InsideQueryNode n = nodeQueue.front();
			nodeQueue.pop();

			// increase count if there's intersection with a leaf
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

				f32 t;
				if (queryRayIntersectsWithTriangle(t, p, dir, a, b, c)) {
					intersections++;
					closestTs.push_back(t);
				}
			}
			else {
				// check intersection with every child overlapping the ray
				const Node& lchild = bvh.nodes[bvh.nodes[n.nodeId].lchildId];
				const Node& rchild = bvh.nodes[bvh.nodes[n.nodeId].lchildId + 1];
				f32 t;
				if (queryRayIntersectsWithBox(t, p, dir, lchild.min, lchild.max)) {
					nodeQueue.push(InsideQueryNode{ bvh.nodes[n.nodeId].lchildId });
				}
				if (queryRayIntersectsWithBox(t, p, dir, rchild.min, rchild.max)) {
					nodeQueue.push(InsideQueryNode{ bvh.nodes[n.nodeId].lchildId + 1 });
				}
			}
		}

		// the point is inside if the number of intersections is odd
		bool inside = intersections % 2;
		return inside;
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
	f32 distanceToBoxSq(const Vec3& p, const Vec3& min, const Vec3& max) {
		Vec3 clamped = Math::max(Math::min(max, p), min);
		return Math::magSq(Math::subtract(clamped, p));
	}
	struct DistanceQueryNode {
		u32 nodeId;
		// distance to the node's bb (the minimum possible distance from the query point to a primitive inside the node)
		f32 distanceSq;
	};
	void findClosestPoint(Vec3& resultPoint, const Tree& bvh, const Vec3& p, f32* vertexPool, const u16* indexPool, const u32 indexCount) {
		// heap is sorted keeping the node with the shortest candidate distance on top
		auto cmp = [](const DistanceQueryNode& a, const DistanceQueryNode& b) { return a.distanceSq > b.distanceSq; };
		std::priority_queue<DistanceQueryNode, tinystl::vector<DistanceQueryNode>, decltype(cmp)> nodeHeap(cmp);
		nodeHeap.push(DistanceQueryNode{ 0, 0.f });
		f32 closestDistanceSq = FLT_MAX;
		Vec3 closestPoint = {};
		u32 closestTriangleId = 0;

		// traverse the heap until the node with the shortest minimum distance possible can't get any better than the current candidate distance
		while (nodeHeap.size() > 0 && nodeHeap.top().distanceSq < closestDistanceSq) {
			DistanceQueryNode n = nodeHeap.top();
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
				const f32 ldistSq = distanceToBoxSq(p, lchild.min, lchild.max);
				const f32 rdistSq = distanceToBoxSq(p, rchild.min, rchild.max);

				// consider each child as long as their minimum possible distance is smaller than our candidate
				if (ldistSq < closestDistanceSq) {
					nodeHeap.push(DistanceQueryNode{ bvh.nodes[n.nodeId].lchildId, ldistSq });
				}
				if (rdistSq < closestDistanceSq) {
					nodeHeap.push(DistanceQueryNode{ bvh.nodes[n.nodeId].lchildId + 1, rdistSq });
				}
			}
		}

		resultPoint = closestPoint;
	}

}; // namespace BVH

#endif // __WASTELADNS_BVH_H__
