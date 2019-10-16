#pragma once

// PERF:

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

	return { data.find(token, 0), endLoc };
}

inline size_t GetBeginOfNextToken(const std::string& data, size_t offset)
{
	static char delimiters[] = " \n\t";

	return data.find_first_not_of(delimiters, offset);
}

inline std::string IncludeRecursively(
	const std::string& data, const uri::Uri& parentPath, std::unordered_set<uri::Uri>& alreadyIncluded)
{
	std::string res = data;

	std::unordered_set<uri::Uri> included = alreadyIncluded;
	auto includeToken = FindTokenLocations(res, "#include", 0);
	while (includeToken.beginLoc != std::string::npos) {
		auto pathBeginPos = GetBeginOfNextToken(res, includeToken.endLoc);

		if (pathBeginPos == std::string::npos || (res[pathBeginPos] != '\"')) {
			LOG_WARN("glsl prepocessor include error in {}, expected tokens: \", include pos: {}",
				uri::GetFilename(parentPath), includeToken.beginLoc);
			// if this fails the include command on
			// the upper level will be replaced by nothing
			// this way the glsl compiler will generate
			// any additional errors
			return {};
		}

		auto expectedClosing = res[pathBeginPos] == '\"' ? '\"' : '<';

		auto pathEndPos = res.find_first_of(expectedClosing, pathBeginPos + 1);
		if (pathEndPos == std::string::npos) {
			LOG_WARN("glsl prepocessor include error in {}, expected token: {}, include pos: {}",
				uri::GetFilename(parentPath), expectedClosing, includeToken.beginLoc);
			return {};
		}

		auto pathNameSize = pathEndPos - pathBeginPos - 1;
		auto path = res.substr(pathBeginPos + 1, pathNameSize);

		uri::Uri parentDir;
		parentDir = uri::GetDir(parentPath);

		std::string filename;
		filename = uri::GetFilename(path);

		// if already included in this recursive path then break;
		if (alreadyIncluded.find(filename) != alreadyIncluded.end()) {
			LOG_WARN("glsl prepocessor include error in {}, recursive inclusion, include pos: {}",
				uri::GetFilename(parentPath), includeToken.beginLoc);
			return {};
		}

		included.insert(filename);

		auto includeePod = AssetManager::GetOrCreateFromParentUri<StringPod>(path, parentDir);
		auto includeeData = includeePod.Lock()->data;

		auto includeeParentPath = (uri::IsUriRelative(path) ? parentDir : "") + path;
		auto injData = IncludeRecursively(includeeData, includeeParentPath, included);
		// inject data onto #include command
		res.erase(includeToken.beginLoc, pathEndPos - includeToken.beginLoc + 1);
		res.insert(includeToken.beginLoc, injData);
		// search next include
		includeToken = FindTokenLocations(res, "#include", 0);
		included = alreadyIncluded;
	}

	return res;
}

// expects #include "...."
inline std::string ProcessIncludeCommands(const std::string& data, const uri::Uri& parentPath, size_t& offset)
{
	// copy
	std::string res = data;

	// search for includes (recursively)
	std::unordered_set<uri::Uri> inc{};
	res = IncludeRecursively(data, parentPath, inc);

	size_t old = std::count(data.begin(), data.end(), '\n');
	size_t newf = std::count(res.begin(), res.end(), '\n');

	offset = newf - old;

	return res;
}
}; // namespace glsl
