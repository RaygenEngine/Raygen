#pragma once
#include "assets/pods/Cubemap.h"
#include "universe/nodes/Node.h"
#include "rendering/scene/Scene.h"

class ReflectionProbeNode : public Node {

	REFLECTED_NODE(ReflectionProbeNode, Node, DF_FLAGS(SkyTexture, AmbientTerm))
	{
		REFLECT_VAR(m_ambientTerm, PropertyFlags::Color).OnDirty(DF::AmbientTerm);
		REFLECT_VAR(m_skybox).OnDirty(DF::SkyTexture);
	}

	PodHandle<Cubemap> m_skybox;

	glm::vec3 m_ambientTerm{ 0.0f, 0.0f, 0.3f };

public:
	[[nodiscard]] PodHandle<Cubemap> GetSkybox() const { return m_skybox; }
	[[nodiscard]] glm::vec3 GetAmbientTerm() const { return m_ambientTerm; }

	void DirtyUpdate(DirtyFlagset flags) override;

	ReflectionProbeNode();
	~ReflectionProbeNode() override;

	void OnBuild() override;

private:
	size_t sceneUid;
	template<typename Lambda>
	void Enqueue(Lambda&& l)
	{
		Scene->EnqueueCmd<SceneReflectionProbe>(sceneUid, l);
	}
};
