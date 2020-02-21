#pragma once
#include <filesystem>
#include <functional>
#include <optional>
namespace fs = std::filesystem;


namespace ImEd {

// A non header only imgui file browser
// Minimal interface for now, expected to be used as a "single instance" that can be used everywhere.

// NOTE: Currently unused. Decided to implement NativeFileDialog (at least for now)
class FileBrowser {
public:
	enum class Mode
	{
		SaveFile,
		LoadFile,
		SelectDirectory
	};

	struct BrowserOperationInfo {
		Mode mode;
		fs::path initialPath{};                          // if left empty to be the current working directory
		std::vector<std::string> fileFilters{};          // eg: ".cpp"
		std::string title{ "Select File##FileBrowser" }; // You can pass ##id from here directly to imgui

		bool allowCreateDirectory{ true };
	};

	// Callback only gets called when the user confirms a selection and nothing happens when he exits
	void OpenDialog(std::function<void(const fs::path&)>&& callback, BrowserOperationInfo&& openInfo = {});

	void Draw();

private:
	struct Entry {
		fs::directory_entry fsEntry;
		fs::path path;
		std::string visibleName; // cached name to avoid wchar to char every frame
		Entry(const fs::directory_entry& e);
	};
	std::optional<Entry> m_parentDirEntry;
	std::vector<Entry> m_directories;
	std::vector<Entry> m_files;
	fs::path m_path{};

	// Generated on ChangeDirectory
	std::string m_currentPathTransient{};

	bool m_isOpen{ false };

	BrowserOperationInfo m_opInfo;

	std::function<void(fs::path)> m_callback;

	bool DrawElement(const Entry& entry);

	void ConfirmSelection(const fs::path& path);
	void RefreshDirectory();
	void ChangeDirectory(const fs::path& newPath);

	void ImguiHeader();
};
} // namespace ImEd
