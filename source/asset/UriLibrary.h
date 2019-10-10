#pragma once

// The central library for the engines Uri convention.
#include "nlohmann/json.hpp"
namespace fs = std::filesystem;


namespace uri {
using Uri = std::string;

namespace detail {
	constexpr char JsonBegin = '{';
	constexpr char JsonEnd = '}';

} // namespace detail
// DOC:

inline bool IsCpu(const Uri& path)
{
	return path.back() == detail::JsonEnd;
}

// WARNING: string view points to the passed 'path'. If path gets moved or deallocated
// the string_view returned from this becomes invalid.
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

// Expects json object, values or arrays are not allowed.
inline Uri MakeChildJson(const Uri& parent, const nlohmann::json& json)
{
	CLOG_ASSERT(!json.is_object(), "Make child json expects a json object.");
	std::string v{ GetDiskPathStrView(parent) };
	v += json.dump();
	return v;
}

inline nlohmann::json GetJson(const Uri& path)
{
	CLOG_ASSERT(!IsCpu(path), "Attempted to get json from a disk path: {}", path);

	size_t startOffset = path.find_first_of(detail::JsonBegin);
	size_t length = path.size() - startOffset;
	std::string_view str{ path.data() + startOffset, length };

	return nlohmann::json::parse(str);
}

// Convert a uri to a path relative the current working directory.
inline const char* ToSystemPath(const Uri& path)
{
	CLOG_ASSERT(IsCpu(path), "Attempted to get system path from a cpu path: {}", path);
	return path.c_str() + 1;
}
} // namespace uri
