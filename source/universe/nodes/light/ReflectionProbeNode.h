#pragma once
#include "assets/pods/EnvironmentMap.h"
#include "universe/nodes/Node.h"
#include "rendering/scene/Scene.h"

class ReflectionProbeNode : public Node {

	REFLECTED_NODE(ReflectionProbeNode, Node, DF_FLAGS(EnvMap, AmbientTerm))
	{
		REFLECT_VAR(m_ambientTerm, PropertyFlags::Color).OnDirty(DF::AmbientTerm);
		REFLECT_VAR(m_environmentMap).OnDirty(DF::EnvMap);
	}

	PodHandle<EnvironmentMap> m_environmentMap;

	// for hacked ambient
	glm::vec3 m_ambientTerm{ 0.0f, 0.0f, 0.3f };

public:
	void DirtyUpdate(DirtyFlagset flags) override;

	ReflectionProbeNode();
	~ReflectionProbeNode() override;

private:
	size_t sceneUid;
	template<typename Lambda>
	void Enqueue(Lambda&& l)
	{
		Scene->EnqueueCmd<SceneReflectionProbe>(sceneUid, l);
	}
};
