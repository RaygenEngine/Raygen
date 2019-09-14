#include "pch.h"

#include "asset/assets/TextAsset.h"

bool TextAsset::Load()
{
	std::ifstream t(m_uri);

	if (!t.is_open())
	{
		LOG_WARN("Unable to open string file, path: {}", m_uri);
		return false;
	}

	t.seekg(0, std::ios::end);
	m_pod->data.reserve(t.tellg());
	
	t.seekg(0, std::ios::beg);
	m_pod->data.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

	return true;
}
