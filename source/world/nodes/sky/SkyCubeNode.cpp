#include "pch.h"

#include "world/nodes/sky/SkyCubeNode.h"
#include "asset/util/ParsingAux.h"
#include "asset/AssetManager.h"

std::string SkyCubeNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--SkyCube " + Node::ToString(verbose, depth);
}
