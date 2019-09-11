#include "pch.h"

#include "assets/other/utf8/StringFile.h"

bool StringFile::Load()
{
	std::ifstream t(m_uri);

	if (!t.is_open())
	{
		LOG_WARN("Unable to open string file, path: {}", m_uri);
		return false;
	}

	t.seekg(0, std::ios::end);
	m_data.reserve(t.tellg());
	t.seekg(0, std::ios::beg);
	m_data.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

	return true;
}

void StringFile::Unload()
{
	std::string().swap(m_data);
}
