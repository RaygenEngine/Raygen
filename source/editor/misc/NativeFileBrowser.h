#pragma once
#include <optional>

namespace ed {
class NativeFileBrowser {
public:
	// CHECK:
	struct ExtensionFilter {
		std::string filter{};

		ExtensionFilter() {}

		// Example "png,jpg;pdf" will enable 3 filters: (png, jpg), (pdf), (*)
		ExtensionFilter(const std::string& filter)
			: filter(filter)
		{
		}
	};


	//
	// This interface can be used like this:
	// if (auto file = SaveFile({ "jpg" })) {
	//    fs::path x = *file; // * operator on optional OR
	//    file->replace_extension(".jpg"); // -> operator
	// }
	//

	// See example usage in NativeFileBrowser.h
	[[nodiscard]] static auto OpenFile(const ExtensionFilter& extensions = {}, const fs::path& initialPath = {})
		-> std::optional<fs::path>;

	// See example usage in NativeFileBrowser.h
	[[nodiscard]] static auto OpenFileMultiple(const ExtensionFilter& extensions = {}, const fs::path& initialPath = {})
		-> std::optional<std::vector<fs::path>>;

	// See example usage in NativeFileBrowser.h
	[[nodiscard]] static auto SelectFolder(const fs::path& initialPath = {}) -> std::optional<fs::path>;

	// See example usage in NativeFileBrowser.h
	[[nodiscard]] static auto SaveFile(const ExtensionFilter& extensions = {}, const fs::path& initialPath = {})
		-> std::optional<fs::path>;
};
} // namespace ed
