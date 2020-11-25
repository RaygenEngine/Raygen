#include "ShaderRegistry.h"

inline ShaderRegistry& ShaderRegistry::Get()
{
	static ShaderRegistry inst = ShaderRegistry();
	return inst;
}

inline PodHandle<ShaderStage> ShaderRegistry::KNode::GetLeafPod() const
{
	CLOG_ABORT(!isLeaf, "ShaderRegistry: Not a leaf!");
	return pod;
}

inline PodHandle<ShaderHeader> ShaderRegistry::KNode::GetHeaderPod() const
{
	CLOG_ABORT(isLeaf, "ShaderRegistry: Not a header file!");
	return pod;
}
