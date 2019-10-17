#pragma once

// WIP:

#include "asset/UriLibrary.h"
#include "asset/AssetManager.h"

#include <string>
#include <unordered_set>

namespace glsl {

struct Token {
	// inclusive
	size_t beginLoc;
	// exclusive
	size_t endLoc;
};

inline Token FindTokenLocations(const std::string& data, const std::string& token, size_t offset)
{
	auto beginLos = data.find(token, offset);
	auto endLoc = beginLos + token.size();

	return { beginLos, endLoc };
}

inline size_t GetBeginOfNextToken(const std::string& data, size_t offset)
{
	static char delimiters[] = " \n\t";

	return data.find_first_not_of(delimiters, offset);
}

inline std::string IncludeRecursively(PodHandle<StringPod> shaderSource, std::vector<BasePodHandle>& includeStack)
{
	includeStack.push_back(shaderSource);
	std::string res = shaderSource.Lock()->data;

	auto includeToken = FindTokenLocations(res, "#include", 0);
	while (includeToken.beginLoc != std::string::npos) {
		auto pathBeginPos = GetBeginOfNextToken(res, includeToken.endLoc);

		if (pathBeginPos == std::string::npos || (res[pathBeginPos] != '\"')) {
			LOG_WARN("glsl prepocessor include error in {}, expected tokens: \", include pos: {}",
				AssetManager::GetPodUri(shaderSource), includeToken.beginLoc);
			// if this fails the include command on
			// the upper level will be replaced by nothing
			// this way the glsl compiler will generate
			// any additional errors
			return {};
		}

		auto expectedClosing = '\"';

		auto pathEndPos = res.find_first_of(expectedClosing, pathBeginPos + 1);
		if (pathEndPos == std::string::npos) {
			LOG_WARN("glsl prepocessor include error in {}, expected token: {}, include pos: {}",
				AssetManager::GetPodUri(shaderSource), expectedClosing, includeToken.beginLoc);
			return {};
		}

		auto pathNameSize = pathEndPos - pathBeginPos - 1;
		auto path = res.substr(pathBeginPos + 1, pathNameSize);

		auto includeePodHandle = AssetManager::GetOrCreateFromParent<StringPod>(path, shaderSource);

		if (std::find(includeStack.begin(), includeStack.end(), includeePodHandle) != includeStack.end()) {
			return fmt::format("#error circular inclusion at '{}', already included '{}'",
				AssetManager::GetPodUri(shaderSource), AssetManager::GetPodUri(includeePodHandle));
		}

		auto includeeData = includeePodHandle.Lock()->data;

		auto includeeUri = AssetManager::GetPodUri(includeePodHandle);


		auto injData = IncludeRecursively(includeePodHandle, includeStack);

		res.erase(includeToken.beginLoc, pathEndPos - includeToken.beginLoc + 1);
		res.insert(includeToken.beginLoc, injData);

		// search next include
		includeToken = FindTokenLocations(res, "#include", 0);
	}

	includeStack.pop_back();

	return res;
}

// expects #include "...."
inline std::string ProcessIncludeCommands(PodHandle<StringPod> source, size_t& offset)
{
	timer::ScopedTimer _("includes");

	std::string data = source.Lock()->data;

	// copy
	std::string res = data;

	// search for includes (recursively)
	std::vector<BasePodHandle> includeStack{};
	res = IncludeRecursively(source, includeStack);

	size_t old = std::count(data.begin(), data.end(), '\n');
	size_t newf = std::count(res.begin(), res.end(), '\n');

	offset = newf - old;

	return res;
}
}; // namespace glsl
