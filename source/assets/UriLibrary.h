#pragma once
#include "core/StringConversions.h"

// The central library for the engines Uri convention.
#include <nlohmann/json_fwd.hpp>

// TEST: lib
// DOC: lib
namespace uri {
using Uri = std::string;

namespace detail {
	constexpr char JsonBegin = '{';
	constexpr char JsonEnd = '}';

} // namespace detail

inline bool IsCpu(const Uri& path)
{
	return !path.empty() && path.back() == detail::JsonEnd;
}

inline bool HasJson(const Uri& path)
{
	return !path.empty() && path.back() == detail::JsonEnd;
}


// WARNING: string view points to the passed 'path'. If path gets moved or deallocated
// the string_view returned from this becomes invalid.
// Strips json metadata from path
inline std::string_view GetDiskPathStrView(const Uri& path)
{
	if (!IsCpu(path)) {
		return std::string_view(path.c_str(), path.size());
	}
	return std::string_view(path.c_str(), path.find_first_of(detail::JsonBegin));
}

inline uri::Uri GetDiskPath(const Uri& path)
{
	uri::Uri s;
	s = GetDiskPathStrView(path);
	return s;
}

// Preserves the '.' from the extension
// String view gets invalidated if the given path is moved or deallocated.
inline std::string_view GetDiskExtension(const Uri& path)
{
	auto diskpath = GetDiskPathStrView(path);

	const auto cutIndex = diskpath.rfind('.'); // Preserve the last '.'
	if (cutIndex == std::string::npos) {
		return {};
	}
	diskpath.remove_prefix(cutIndex); // resolve parents directory
	return diskpath;
}

inline bool MatchesExtension(const Uri& path, const std::string& ext)
{
	return GetDiskExtension(path) == ext;
}

inline std::string_view GetFilename(const Uri& path)
{
	auto diskView = GetDiskPathStrView(path);
	auto loc = diskView.find_last_of('/');

	if (loc == std::string::npos) {
		return diskView;
	}

	return diskView.substr(loc + 1);
}

// Also strips json
inline std::string_view GetFilenameNoExt(const Uri& path)
{
	auto diskView = GetDiskPathStrView(path);
	auto loc = diskView.find_last_of('/');

	if (loc != std::string::npos) {
		diskView = diskView.substr(loc + 1);
	}

	loc = diskView.rfind('.');
	if (loc != std::string::npos) {
		diskView.remove_suffix(diskView.size() - loc);
	}
	return diskView;
}

inline std::string_view StripExt(const Uri& path)
{
	auto diskView = GetDiskPathStrView(path);
	auto loc = diskView.rfind('.');
	if (loc != std::string::npos) {
		diskView.remove_suffix(diskView.size() - loc);
	}
	return diskView;
}

inline std::string_view GetDir(const Uri& path)
{
	auto diskView = GetDiskPathStrView(path);
	auto loc = diskView.find_last_of('/');

	if (loc == std::string::npos) {
		return "";
	}

	return diskView.substr(0, loc + 1);
}

inline bool IsUri(const std::string& path)
{
	if (path.empty()) {
		return false;
	}

	return path[0] != '/';
}


// Expects json object, values or arrays are not allowed.
Uri MakeChildJson(const Uri& parent, const nlohmann::json& json);


// inline nlohmann::json GetJson(const Uri& path)
//{
//	CLOG_ABORT(!IsCpu(path), "Attempted to get json from a disk path: {}", path);
//
//	size_t startOffset = path.find_first_of(detail::JsonBegin);
//	size_t length = path.size() - startOffset;
//	std::string_view str{ path.data() + startOffset, length };
//
//	return nlohmann::json::parse(str);
//}

// Convert a uri to a path relative the current working directory.
inline const char* ToSystemPath(const Uri& path)
{
	CLOG_ABORT(IsCpu(path), "Attempted to get system path from a cpu path: {}", path);
	if (path.starts_with('/')) {
		return path.c_str() + 1;
	}
	return path.c_str();
}

inline std::string SystemToUri(fs::path& path)
{
	auto strPath = "/" + fs::relative(path).string();
	for (int32 i = 0; i < strPath.size(); ++i) {
		if (strPath[i] == '\\') {
			strPath[i] = '/';
		}
	}
	return strPath;
}

// Extracts the number from the suffix of the filename or 0 if no suffix found
// (eg: Mesh_13 would reutrn 13 and outActualName: "Mesh_")
inline size_t DetectCountFromPath(const Uri& path, std::string_view& outActualName)
{
	size_t number = 0;
	outActualName = path;

	if (path.empty()) {
		return 0;
	}

	const char* begin = path.c_str();
	const char* end = path.c_str() + path.size();

	for (const char* c = end - 1; c != begin; --c) {
		bool isDigit = *c >= '0' && *c <= '9';
		if (!isDigit) {
			if (c == end - 1) {
				return 0;
			}
			auto numview = std::string_view(c + 1, end - (c + 1));
			number = str::fromStrView<size_t>(numview);
			outActualName = std::string_view(begin, (end - begin) - numview.size());
			break;
		}
	}
	return number;
}

} // namespace uri
