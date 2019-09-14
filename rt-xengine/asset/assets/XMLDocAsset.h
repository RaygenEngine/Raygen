#pragma once

#include "asset/Asset.h"
#include "asset/pods/XMLDocPod.h"

#include "tinyxml2/tinyxml2.h"

class XMLDocAsset : public PodedAsset<XMLDocPod>
{

public:
	XMLDocAsset(const fs::path& path)
		: PodedAsset(path) {}
	~XMLDocAsset() = default;

	const tinyxml2::XMLElement* GetRootElement() const { return m_pod->document.RootElement(); }

protected:
	bool Load() override;
};
