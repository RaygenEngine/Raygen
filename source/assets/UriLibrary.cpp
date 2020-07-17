#include "pch.h"

#include "UriLibrary.h"

#include <nlohmann/json.hpp>

namespace uri {
Uri MakeChildJson(const Uri& parent, const nlohmann::json& json)
{
	CLOG_ABORT(!json.is_object(), "Make child json expects a json object.");
	std::string v{ GetDiskPathStrView(parent) };
	v += json.dump();
	return v;
}
} // namespace uri
