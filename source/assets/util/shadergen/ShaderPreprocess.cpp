#include "ShaderPreprocess.h"

#include "assets/util/shadergen/ShaderGen.h"

namespace shd {
// Include specification:
//
// Only supported include formats are:
// * "#include" MUST be at the start of the line
// * only the character " can be used between the directory
// * there must be EXACTLY A SINGLE space between #include and the character "
// * No #include can be used after a #pragma begin
//
// Anything that does not adhere to the above should be considered ub at this point.
// Some basic errors may be reported, many will be a failed to find include file
//

//
// All includes are automatically assumed to be #pragma once
// ALL paths are relative to a SINGLE root directory. (ie #include "abc.glsl" matches EXACTLY one file ALWAYS, no side
// The root include directory path is: CURRENT_WORKING_DIR/{ShaderRegistry::c_shaderIncludeDir}
// by side includes in subfolders)
// All headers are expected to be ".glsl" any non .glsl header will be reported as a warning
// Relative Directories or directories that contain '.' are not supported
//

std::vector<std::pair<int, std::string>> ExtractIncludes(const std::string& code)
{
	using namespace std::literals;
	if (code.empty()) {
		return {};
	}

	std::vector<std::pair<int, std::string>> includes;

	int32 lineNum = 0;
	str::splitForEach(
		code,
		[&](std::string_view line) {
			if (line.starts_with("#include \"")) {
				// Handle include
				constexpr auto includeBeginOffset = "#include \""sv.size();
				line = line.substr(includeBeginOffset);
				line = line.substr(0, line.find_first_of('"'));
				if (!line.ends_with(".glsl")) {
					LOG_WARN(
						"Found non .glsl include in shader. Non .glsl includes are not handled by the internal "
						"preprocessor.");
				}
				else {
					includes.emplace_back(lineNum, line);
				}
			}
			lineNum++;
			if (line.starts_with("#pragma begin")) {
				return false;
			}
			return true;
		},
		"\n");

	return includes;
}

std::string PreprocessCode(
	const std::string& code, std::vector<std::tuple<int32, std::string, ShaderRegistry::KNode*>>& outIncludesReplaced)
{
	// TIMER_STATIC_SCOPE("Preprocess Code");
	// ^^ results to around 50ms on boot up time 3/12/2020.

	std::stringstream result;
	using namespace std::literals;
	if (code.empty()) {
		return {};
	}
	constexpr auto nl = '\n';
	auto delim = "\n"sv;
	int32 lineNum = 0;

	int32 includeIndex = 1;

	for (auto first = code.data(), second = code.data(), last = first + code.size(); second != last && first != last;
		 first = second + 1) {
		second = std::find_first_of(first, last, delim.begin(), delim.end());
		if (first == second) {
			continue;
		}
		auto lineView = std::string_view(first, second - first);
		lineNum++;

		if (lineView.starts_with("#include \"")) {
			// Handle include
			constexpr auto includeBeginOffset = "#include \""sv.size();
			auto parsedView = lineView.substr(includeBeginOffset);
			parsedView = parsedView.substr(0, parsedView.find_first_of('"'));
			if (!parsedView.ends_with(".glsl")) {
				LOG_WARN(
					"Found non .glsl include in shader. Non .glsl includes are not handled by the internal "
					"preprocessor.");
				result << lineView << nl;
			}
			else {
				auto node = ShaderRegistry::FindOrAdd(std::string(parsedView));
				if (!node) {
					result << lineView << nl;
					continue;
				}
				outIncludesReplaced.emplace_back(includeIndex++, parsedView, node);
				result << "#line " << (includeIndex * shd::c_errorLineModifier) + 1 << nl;
				result << node->processedCode << nl;
				result << "#line " << lineNum << nl;
			}
		}
		else if (lineView.starts_with("#pragma begin")) {
			result << std::string_view(first, last - first);
			break;
		}
		else {
			// PERF: can be done faster by splitting at '#' chars but the code would become slightly more complex
			result << lineView << nl;
		}
	}
	return code;

	//	return result.str();
}
} // namespace shd
