#pragma once

#include "asset/AssetManager.h"
#include "reflection/ReflectionTools.h"

#include <fstream>


namespace GenericJsonLoader {
template<typename T>
inline void Load(T* pod, const uri::Uri& path)
{
	using json = nlohmann::json;
	std::ifstream file(uri::ToSystemPath(path));

	CLOG_ABORT(!file.is_open(), "Failed to open generic json file at: {}", path);

	int32 firstChar = file.peek();
	CLOG_ABORT(firstChar == EOF, "Found empty shader file: {} No shaders loaded", uri::ToSystemPath(path));

	nlohmann::json j;
	file >> j;

	refltools::JsonToPropVisitor_WithRelativePath visitor(j, path);
	refltools::CallVisitorOnEveryProperty(pod, visitor);
}
} // namespace GenericJsonLoader
