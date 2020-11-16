#include "Test.h"

#include "core/math-ext/BVH.h"

TEST("BVH RayCast Simple")
{
	using math::BVH;
	using math::AABB;


	AABB cube{ { -1, -1, -1 }, { 1, 1, 1 } };

	BVH tree;


	auto hitCube = cube.Transform(math::transformMat2({ 10.f, 10.f, 10.f }));
	tree.InsertLeaf(2, hitCube);
	tree.InsertLeaf(3, cube.Transform(math::transformMat2({ -10.f, 10.f, 10.f })));
	tree.InsertLeaf(4, cube.Transform(math::transformMat2({ -10.f, 10.f, 10.f })));
	tree.InsertLeaf(5, cube.Transform(math::transformMat2({ -10.f, 10.f, 10.f })));
	tree.InsertLeaf(1, cube);

	auto result = tree.RayCast({ 6, 6, 6 }, { 14, 14, 14 });

	REQ(result.distanceSqToHitObject.empty() != true);

	// TODO: fix hitPoint
	// REQ(math::equals(result.distanceSqToHitObject.begin()->first, 4.f * 4.f));

	REQ(result.distanceSqToHitObject.begin()->second == 2);
}


TEST("BVH Create")
{
	using math::BVH;
	using math::AABB;


	AABB unitBox{ { 0, 0, 0 }, { 1, 1, 1 } };

	BVH tree;

	tree.InsertLeaf(1, unitBox.Transform(math::transformMat2({ 2, 2, 2 })));
	tree.InsertLeaf(2, unitBox);
	tree.InsertLeaf(3, unitBox.Transform(math::transformMat2({}, {}, { 3.f, 3.f, 3.f })));
	tree.InsertLeaf(4, unitBox.Transform(math::transformMat2({}, {}, { 1.f, 3.f, 1.f })));
	tree.InsertLeaf(5, unitBox.Transform(math::transformMat2({ 4, -4, 0 }, {}, { 1.f, 3.f, 1.f })));
	//	tree.RayCast({ 5, -5, 0 }, { 0, 0, 0 });
}
