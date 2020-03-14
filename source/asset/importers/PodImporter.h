#pragma once
#include "reflection/TypeId.h"
#include "asset/PodHandle.h"

class PodImporterBase {
	std::string_view m_name;
	std::vector<const char*> m_supportedExtensions{};
	mti::TypeId m_primaryPodType;

protected:
	PodImporterBase(mti::TypeId type, const std::vector<const char*>& supportedExt, std::string_view name)
		: m_supportedExtensions(supportedExt)
		, m_name(name)
		, m_primaryPodType(type)
	{
	}

public:
	std::vector<const char*>& GetSupportedExtensions() { return m_supportedExtensions; };
	[[nodiscard]] virtual mti::TypeId GetPrimaryPodType() const { return m_primaryPodType; }


	virtual BasePodHandle Import(const fs::path& path) = 0;
	virtual ~PodImporterBase() = default;
};

template<typename T>
struct PodImporter : public PodImporterBase {
public:
	PodImporter(const std::vector<const char*>& supportedExt, std::string_view name)
		: PodImporterBase(mti::GetTypeId<T>(), supportedExt, name)
	{
	}

	virtual ~PodImporter() = default;
};
