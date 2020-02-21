#pragma once
#include <optional>

namespace ed {
class NativeFileBrowser {
public:
	struct ExtensionFilter {
		std::string filter{};

		ExtensionFilter() {}

		// Example "png,jpg;pdf" will enable 3 filters: (png, jpg), (pdf), (*)
		ExtensionFilter(const std::string& filter)
			: filter(filter)
		{
		}
	};

	[[nodiscard]] static auto OpenFile(const ExtensionFilter& extensions = {},
		const fs::path& initialPath = fs::current_path()) -> std::optional<fs::path>;

	[[nodiscard]] static auto OpenFileMultiple(const ExtensionFilter& extensions = {},
		const fs::path& initialPath = fs::current_path()) -> std::vector<fs::path>;

	[[nodiscard]] static auto SelectFolder(const fs::path& initialPath = fs::current_path()) //
		-> std::optional<fs::path>;

	[[nodiscard]] static auto SaveFile(const ExtensionFilter& extensions = {},
		const fs::path& initialPath = fs::current_path()) -> std::optional<fs::path>;
};
} // namespace ed
