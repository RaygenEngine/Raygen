#pragma once
#include "core/math-ext/AABB.h"

#include <stack>
#include <queue>

namespace math {
template<typename T>
struct BVH {
	using DataT = T;

	static constexpr int32 nullIndex = -1;

	int32 rootIndex{ nullIndex };

	struct Node {
		Node() = default;
		Node(AABB b, DataT d)
			: aabb(b)
			, data(d)
		{
		}
		AABB aabb;
		DataT data;

		int32 parentIndex{ nullIndex };
		int32 child1{ nullIndex };
		int32 child2{ nullIndex };
		bool isLeaf;
	};

	struct RayCastResult {
		std::map<float, DataT> distanceSqToHitObject;
	};

	std::vector<BVH::Node> nodes;

	[[nodiscard]] RayCastResult RayCastDirection(glm::vec3 start, glm::vec3 direction, float distance) const
	{
		return RayCast(start, glm::normalize(direction) * distance);
	}

	// TODO: anyhit
	// PERF: really basic impl
	[[nodiscard]] RayCastResult RayCast(glm::vec3 start, glm::vec3 end) const
	{
		if (rootIndex == nullIndex) {
			return {};
		}

		RayCastResult results;
		std::stack<int32> stack;

		stack.push(rootIndex);

		while (!stack.empty()) {
			const BVH::Node& node = nodes[stack.top()];
			stack.pop();

			glm::vec3 hitPoint;
			if (!node.aabb.OverlapsHitPoint(start, end, hitPoint)) {
				continue;
			}

			if (node.isLeaf) {
				// Overlapping leaf, add to results
				// WIP: should implement DataT intersect for exact geometry casting)
				// WIP: proper calculation of distance (detect hit point / ... )
				results.distanceSqToHitObject.emplace(glm::distance2(start, hitPoint), node.data);
			}
			else {
				stack.push(node.child1);
				stack.push(node.child2);
			}
		}

		return results;
	}

private:
	int32 AllocateLeaf(DataT data, AABB box)
	{
		int32 leafIndex = nodes.size();
		auto& node = nodes.emplace_back(box, data);
		node.isLeaf = true;
		return leafIndex;
	}

	int32 AllocateInternal()
	{
		int32 index = nodes.size();
		auto& node = nodes.emplace_back();
		node.isLeaf = false;
		return index;
	}

	int32 FindBestSibling(int32 node)
	{
		using pqdata = std::tuple<float, int32>;
		// std::priority_queue<pqdata, std::vector<pqdata>,
		//	decltype([](const pqdata& lhs, const pqdata& rhs) -> bool { return lhs.first < rhs.first; })>
		//	queue;

		// PERF: algorithm: use inherited cost only?

		// Perform bfs with a surface area heuristic
		std::priority_queue<pqdata> queue;

		// PERF: optimise this data into the priority_queue? (it may be faster)
		std::vector<float> sumDeltaSurfaceAreaTo(nodes.size(), 0.f);

		auto queue_push = [&](int32 nodeIndex) {
			float area = nodes[nodeIndex].aabb.Union(nodes[node].aabb).GetArea();

			float sumDeltaPrevSurfaceArea = 0.f;

			// PERF: manually handle root and skip this check.
			if (nodeIndex == nullIndex) [[likely]] {
				sumDeltaPrevSurfaceArea = sumDeltaSurfaceAreaTo[nodes[nodeIndex].parentIndex];
			}

			queue.push({ area + sumDeltaPrevSurfaceArea, nodeIndex });


			float originalArea = nodes[nodeIndex].aabb.GetArea();
			sumDeltaSurfaceAreaTo[nodeIndex] = sumDeltaPrevSurfaceArea + (area - originalArea);
		};

		float surfaceAdded = nodes[node].aabb.GetArea();

		float bestCost = 999999999.f;
		int32 bestIndex = rootIndex;
		// Push root
		queue_push(rootIndex);

		while (!queue.empty()) {
			auto [cost, currentIndex] = queue.top();
			queue.pop();

			if (cost < bestCost) {
				bestCost = cost;
				bestIndex = currentIndex;
			}


			if ((!nodes[currentIndex].isLeaf) && sumDeltaSurfaceAreaTo[currentIndex] + surfaceAdded < bestCost) {
				queue_push(nodes[currentIndex].child1);
				queue_push(nodes[currentIndex].child2);
			}
		}
		return bestIndex;
	}

	void RotateRebalance(int32 nodeIndex)
	{
		//      A
		//    /   \
		//   B     C
		//  / \   / \
		//  D E   F  G
		//
		// nodeIndex is A
		// Possible rotations are:
		// B <-> F
		// B <-> G
		// C <-> D
		// C <-> E

		// Determine costs and find best rotation
		auto& nodeA = nodes[nodeIndex];

		int32 nodeBIndex = nodeA.child1;
		int32 nodeCIndex = nodeA.child2;

		auto& nodeB = nodes[nodeA.child1];
		auto& nodeC = nodes[nodeA.child2];

		int32 nodeDIndex = nodeB.child1;
		int32 nodeEIndex = nodeB.child2;
		int32 nodeFIndex = nodeC.child1;
		int32 nodeGIndex = nodeC.child2;


		float cd_cost = 1.f;
		float ce_cost = 1.f;

		if (!nodeB.isLeaf) {
			float b_cost = nodeB.aabb.GetArea();

			// Costs (relative to the "original" cost. aka no rotation cost)

			cd_cost = nodeC.aabb.Union(nodes[nodeEIndex].aabb).GetArea() - b_cost;
			ce_cost = nodeC.aabb.Union(nodes[nodeDIndex].aabb).GetArea() - b_cost;
		}

		float bf_cost = 1.f;
		float bg_cost = 1.f;

		//
		if (!nodeC.isLeaf) {
			float c_cost = nodeC.aabb.GetArea();

			bf_cost = nodeB.aabb.Union(nodes[nodeGIndex].aabb).GetArea() - c_cost;
			bg_cost = nodeB.aabb.Union(nodes[nodeFIndex].aabb).GetArea() - c_cost;
		}


		int32 minIndex = 0;
		float minCost = bf_cost;

		if (bg_cost < minCost) {
			minIndex = 1;
			minCost = bg_cost;
		}
		if (cd_cost < minCost) {
			minIndex = 2;
			minCost = cd_cost;
		}
		if (ce_cost < minCost) {
			minIndex = 3;
			minCost = ce_cost;
		}

		// No rotation gives gain, do not rotate
		if (minCost >= 0) {
			return;
		}

		auto swap = [&](int32 child, int32 grandchild) {
			auto otherChild = nodes[grandchild].parentIndex;
			nodes[child].parentIndex = nodes[grandchild].parentIndex;
			nodes[grandchild].parentIndex = nodeIndex;


			if (nodeA.child1 == child) {
				nodeA.child1 = grandchild;
			}
			else {
				nodeA.child2 = grandchild;
			}

			if (nodes[otherChild].child1 == grandchild) {
				nodes[otherChild].child1 = child;
			}
			else {
				nodes[otherChild].child2 = child;
			}

			nodes[otherChild].aabb = nodes[nodes[otherChild].child1].aabb.Union(nodes[nodes[otherChild].child2].aabb);
		};

		switch (minIndex) {
			case 0:
				// BF
				swap(nodeBIndex, nodeFIndex);
				break;
			case 1:
				// BG
				swap(nodeBIndex, nodeGIndex);
				break;
			case 2:
				// CD
				swap(nodeCIndex, nodeDIndex);
				break;
			case 3:
				// CE
				swap(nodeCIndex, nodeEIndex);
				break;
		}
	}

public:
	void InsertLeaf(DataT data, AABB box)
	{
		int32 leafIndex = AllocateLeaf(data, box);
		if (leafIndex == 0) {
			rootIndex = leafIndex;
			return;
		}

		// Find sibling
		int32 siblingIndex = FindBestSibling(leafIndex);

		// Create Parent
		{
			int32 oldParent = nodes[siblingIndex].parentIndex;
			int32 newParent = AllocateInternal();

			nodes[newParent].parentIndex = oldParent;
			nodes[newParent].aabb = nodes[siblingIndex].aabb.Union(box);

			// Handle edge cases for parent creation
			if (oldParent != nullIndex) {
				// Sibling selected was not root
				if (nodes[oldParent].child1 == siblingIndex) {
					nodes[oldParent].child1 = newParent;
				}
				else {
					nodes[oldParent].child2 = newParent;
				}

				nodes[newParent].child1 = siblingIndex;
				nodes[newParent].child2 = leafIndex;
				nodes[siblingIndex].parentIndex = newParent;
				nodes[leafIndex].parentIndex = newParent;
			}
			else {
				// Sibling selected was root
				nodes[newParent].child1 = siblingIndex;
				nodes[newParent].child2 = leafIndex;
				nodes[siblingIndex].parentIndex = newParent;
				nodes[leafIndex].parentIndex = newParent;
				rootIndex = newParent;
			}
		}


		// Refit parent aabbs
		int index = nodes[leafIndex].parentIndex;
		while (index != nullIndex) {
			int32 child1 = nodes[index].child1;
			int32 child2 = nodes[index].child2;

			nodes[index].aabb = nodes[child1].aabb.Union(nodes[child2].aabb);

			RotateRebalance(index);

			index = nodes[index].parentIndex;
		}
	}
};

} // namespace math
