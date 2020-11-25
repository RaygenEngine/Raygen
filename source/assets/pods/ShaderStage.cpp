#include "ShaderStage.h"

#include "core/StringUtl.h"

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

void ShaderStage::PreprocessIncludes()
{
	using namespace std::literals;
	if (code.empty()) {
		return;
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
				includes.emplace_back(line);
			}
			if (line.starts_with("#pragma begin")) {
				return false;
			}
			return true;
		},
		"\n");


	std::string report = "Includes found: \n";
	for (auto& str : includes) {
		report += "\t" + str + "\n";
	}

	LOG_REPORT("{}", report);
}
