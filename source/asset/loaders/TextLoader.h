#pragma once

#include "asset/pods/StringPod.h"

namespace TextLoader
{
	inline bool Load(StringPod* pod, const fs::path& path)
	{
		std::ifstream t(path);

		if (!t.is_open())
		{
			LOG_WARN("Unable to open string file, path: {}", path);
			return false;
		}

		t.seekg(0, std::ios::end);
		pod->data.reserve(t.tellg());

		t.seekg(0, std::ios::beg);
		pod->data.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

		return true;
	}
};

