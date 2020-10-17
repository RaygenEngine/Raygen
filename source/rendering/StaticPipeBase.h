#pragma once
// Fwd declared here for subclasses
struct SceneRenderDesc;

namespace vl {
// Follows RAII rules, use custom constructor / destructor if you need more init/cleanup
class StaticPipeBase {
private:
	friend class StaticPipes;
	vk::UniquePipelineLayout m_layout;
	vk::UniquePipeline m_pipeline;

public:
	StaticPipeBase(StaticPipeBase const&) = delete;
	StaticPipeBase(StaticPipeBase&&) = delete;
	StaticPipeBase& operator=(StaticPipeBase const&) = delete;
	StaticPipeBase& operator=(StaticPipeBase&&) = delete;


protected:
	virtual vk::UniquePipelineLayout MakePipelineLayout() = 0;
	virtual vk::UniquePipeline MakePipeline() = 0;

public:
	StaticPipeBase() = default;
	virtual ~StaticPipeBase() = default;

	vk::Pipeline pipeline() const { return *m_pipeline; }
	vk::PipelineLayout layout() const { return *m_layout; }
};
} // namespace vl
