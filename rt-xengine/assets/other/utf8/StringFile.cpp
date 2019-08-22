#include "pch.h"
#include "StringFile.h"

#include "assets/DiskAssetManager.h"

#include <fstream>

namespace Assets
{
	StringFile::StringFile(DiskAssetManager* context)
		: DiskAsset(context)
	{
	}

	bool StringFile::Load(const std::string& path)
	{
		SetIdentificationFromPath(path);

		std::ifstream t(path);

		if (!t.is_open())
		{
			RT_XENGINE_LOG_WARN("Unable to open string file, path: {}", path);
			return false;
		}

		t.seekg(0, std::ios::end);
		m_data.reserve(t.tellg());
		t.seekg(0, std::ios::beg);
		m_data.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

		return true;
	}

	void StringFile::Clear()
	{
		std::string().swap(m_data);
	}
}
