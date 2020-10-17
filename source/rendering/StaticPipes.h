#pragma once

#include "rendering/StaticPipeBase.h"

#include <concepts>

namespace vl {

template<typename T>
concept StaticPipeClass = requires(T instance)
{
	{
		instance.MakePipeline()
	}
	->std::convertible_to<vk::UniquePipeline>;

	{
		instance.MakePipelineLayout()
	}
	->std::convertible_to<vk::UniquePipelineLayout>;

	requires std::is_default_constructible_v<T>;
	requires std::has_virtual_destructor_v<T>;
	requires std::is_base_of_v<StaticPipeBase, T>;
};

class StaticPipes {
public:
	StaticPipes(StaticPipes const&) = delete;
	StaticPipes(StaticPipes&&) = delete;
	StaticPipes& operator=(StaticPipes const&) = delete;
	StaticPipes& operator=(StaticPipes&&) = delete;

private:
	StaticPipes() = default;

	std::unordered_map<size_t, UniquePtr<StaticPipeBase>> m_pipes;

	static StaticPipes& GetInstance()
	{
		static StaticPipes instance;
		return instance;
	}

	template<StaticPipeClass T>
	void InternalInit()
	{
		CLOG_ABORT(m_pipes.contains(mti::GetHash<T>()), "Attempting to double init static pipe: {}", mti::GetName<T>());
		auto& entry = m_pipes[mti::GetHash<T>()];

		entry = std::make_unique<T>();
		entry->m_layout = entry->MakePipelineLayout();
		entry->m_pipeline = entry->MakePipeline();
	}

	vk::Pipeline GetPipe(size_t hash) { return m_pipes.at(hash)->pipeline(); }
	vk::PipelineLayout GetLayout(size_t hash) { return m_pipes.at(hash)->layout(); }

	void InternalInitRegistered();

	const StaticPipeBase* Get(size_t hash) { return m_pipes.at(hash).get(); }

public:
	template<StaticPipeClass T>
	static void Init()
	{
		GetInstance().InternalInit<T>();
	}


	static void DestroyAll() { GetInstance().m_pipes.clear(); }

	template<StaticPipeClass T>
	static const T& Get()
	{
		return *static_cast<const T*>(GetInstance().Get(mti::GetHash<T>()));
	}


	template<StaticPipeClass T>
	static vk::Pipeline GetPipeline()
	{
		return GetInstance().GetPipe(mti::GetHash<T>());
	}

	template<StaticPipeClass T>
	static void Recompile()
	{
		auto& inst = *GetInstance().m_pipes.at(mti::GetHash<T>());
		inst.m_pipeline = inst.MakePipeline();
	}


	static void InitRegistered() { return GetInstance().InternalInitRegistered(); }


	template<StaticPipeClass T>
	static vk::PipelineLayout GetLayout()
	{
		return GetInstance().GetLayout(mti::GetHash<T>());
	}
};
} // namespace vl
