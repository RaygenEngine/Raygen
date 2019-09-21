#include "pch.h"

#include "world/nodes/geometry/GeometryNode.h"
#include "world/World.h"
#include "asset/AssetManager.h"
#include "asset/util/ParsingAux.h"

std::string GeometryNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--TMgeometry " + Node::ToString(verbose, depth);
}