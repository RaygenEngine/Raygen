#pragma once
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/scene/Scene.h"

namespace vl {


class PtBase {

public:
	PtBase() = default;

	// Runs once after all technique registrations are finished
	virtual void Prepare(){};

	virtual void Draw(vk::CommandBuffer cmdBuffer, SceneRenderDesc<SceneCamera>& sceneDesc, uint32 frameIndex) = 0;


	PtBase(const PtBase&) = delete;
	PtBase(PtBase&&) = default;
	PtBase& operator=(const PtBase&) = delete;
	PtBase& operator=(PtBase&&) = default;

	virtual ~PtBase() = default;

	// TODO: Reflected settings struct
};


// TODO: Proper concept
template<typename T>
concept CPostTech = requires
{
	std::is_base_of_v<PtBase, T>;
};

// Base class for a the most common use case of PPT setup.
// TODO: Automated descriptor sets & binding without virtual call
class PtBase_SinglePipeline : public PtBase {


public:
	// Initialize the pipline layout on constructor
	PtBase_SinglePipeline() = default;

	virtual void Prepare()
	{
		MakeLayout();
		MakePipeline();
	};


protected:
	// Not used automatically (yet) just here for convinience and future features.
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	// Make the pipeline layout here. This will be called once after PPT registration finishes
	virtual void MakeLayout() = 0;

	// Make the pipeline here, will be called on shader recompiles (WIP)
	// MakeLayout will get called at least once before this.
	virtual void MakePipeline() = 0;

	virtual void Draw(vk::CommandBuffer cmdBuffer, SceneRenderDesc<SceneCamera>& sceneDesc, uint32 frameIndex) = 0;

	~PtBase_SinglePipeline() override = default;


	void Utl_CreatePipeline(
		GpuAsset<Shader>& shader, vk::PipelineColorBlendStateCreateInfo colorBlending, uint32 subpassIndex = 0);
};


} // namespace vl
