#pragma once

#include <concepts>

namespace vl {

template<typename T>
concept StaticPipeClass = requires
{
	{
		T::MakePipeline()
	}
	->std::convertible_to<vk::UniquePipeline>;

	{
		T::MakePipelineLayout()
	}
	->std::convertible_to<vk::UniquePipelineLayout>;

	// TODO: Finish concept
};

class StaticPipes {
public:
	StaticPipes(StaticPipes const&) = delete;
	StaticPipes(StaticPipes&&) = delete;
	StaticPipes& operator=(StaticPipes const&) = delete;
	StaticPipes& operator=(StaticPipes&&) = delete;

private:
	StaticPipes() = default;

	struct Entry {
		vk::UniquePipeline pipeline;
		vk::UniquePipelineLayout pipelineLayout;
	};

	std::unordered_map<size_t, Entry> m_pipes;


	static StaticPipes& GetInstance()
	{
		static StaticPipes instance;
		return instance;
	}

	template<StaticPipeClass T>
	void InternalInit()
	{
		auto& entry = m_pipes[mti::GetHash<T>()];

		entry.pipelineLayout = T::MakePipelineLayout();
		entry.pipeline = T::MakePipeline();
	}

	vk::Pipeline GetPipe(size_t hash) { return *m_pipes.at(hash).pipeline; }
	vk::PipelineLayout GetLayout(size_t hash) { return *m_pipes.at(hash).pipelineLayout; }

	void InternalInitRegistered();

public:
	template<StaticPipeClass T>
	static void Init()
	{
		GetInstance().InternalInit<T>();
	}


	static void DestroyAll() { GetInstance().m_pipes.clear(); }


	template<StaticPipeClass T>
	static vk::Pipeline Get()
	{
		return GetInstance().GetPipe(mti::GetHash<T>());
	}

	static void InitRegistered() { return GetInstance().InternalInitRegistered(); }


	template<StaticPipeClass T>
	static vk::PipelineLayout GetLayout()
	{
		return GetInstance().GetLayout(mti::GetHash<T>());
	}
};
} // namespace vl
