#include "ShaderPreprocess.h"


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

std::vector<std::string> ExtractIncludes(const std::string& code)
{
	using namespace std::literals;
	if (code.empty()) {
		return {};
	}

	std::vector<std::string> includes;

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
					includes.emplace_back(line);
				}
			}
			if (line.starts_with("#pragma begin")) {
				return false;
			}
			return true;
		},
		"\n");

	return includes;
}
} // namespace shd
