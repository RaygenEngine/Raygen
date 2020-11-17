#include "Test.h"

#include "core/math-ext/BVH.h"

using math::BVH;
using math::AABB;

using BVHTree = BVH<int32>;

const AABB cube{ { -1, -1, -1 }, { 1, 1, 1 } };


TEST("BVH RayCast Diagonial")
{
	BVHTree tree;


	auto hitCube = cube.Transform(math::transformMat2({ 10.f, 10.f, 10.f }));
	tree.InsertLeaf(2, hitCube);
	tree.InsertLeaf(3, cube.Transform(math::transformMat2({ -10.f, 10.f, 10.f })));
	tree.InsertLeaf(4, cube.Transform(math::transformMat2({ -10.f, 10.f, 10.f })));
	tree.InsertLeaf(1, cube);

	auto result = tree.RayCast({ 6, 6, 6 }, { 14, 14, 14 });

	REQ(result.distanceSqToHitObject.empty() != true);
	// TODO: fix hitPoint
	// REQ(math::equals(result.distanceSqToHitObject.begin()->first, 4.f * 4.f));
	REQ(result.distanceSqToHitObject.begin()->second == 2);
}

TEST("BVH RayCast Empty Tree")
{
	BVHTree tree;
	auto result = tree.RayCast({}, glm::vec3(1, 0, 0));
	REQ(result.distanceSqToHitObject.empty());
}

TEST("BVH RayCast Same Point")
{
	BVHTree tree;

	tree.InsertLeaf(1, cube);
	auto result = tree.RayCast(glm::vec3(3, 0, 0), glm::vec3(3, 0, 0));
	REQ(result.distanceSqToHitObject.empty());
	result = tree.RayCast({}, {});
	REQ(result.distanceSqToHitObject.size() > 0);
}


TEST("BVH RayCast")
{
	BVHTree tree;
	tree.InsertLeaf(2, cube);

	auto result = tree.RayCast({ 6, 6, 6 }, { 14, 14, 14 });
}
