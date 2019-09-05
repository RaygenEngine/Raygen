#include "pch.h"

#include "assets/other/utf8/StringFile.h"

namespace Assets
{
	StringFile::StringFile(EngineObject* pObject, const std::string& path)
		: DiskAsset(pObject, path)
	{
	}

	bool StringFile::Load(const std::string& path)
	{
		std::ifstream t(path);

		if (!t.is_open())
		{
			LOG_WARN("Unable to open string file, path: {}", path);
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
