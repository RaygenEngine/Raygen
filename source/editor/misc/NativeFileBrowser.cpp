#include "pch.h"
#include "editor/misc/NativeFileBrowser.h"
#include "engine/Logger.h"
#include <nativefiledialog/src/include/nfd.h>

namespace {

fs::path ToForwardSlashPath(nfdchar_t* txt)
{
	nfdchar_t* ptr = txt;
	while (*ptr != '\0') {
		if (*ptr == '\\') {
			*ptr = '/';
		}
		ptr++;
	}
	return { txt };
}

} // namespace
namespace ed {
std::optional<fs::path> NativeFileBrowser::OpenFile(const ExtensionFilter& extensions, const fs::path& initialPath)
{
	nfdchar_t* outPath = NULL;
	nfdresult_t result = NFD_OpenDialog(extensions.filter.c_str(), U8(initialPath.u8string().c_str()), &outPath);

	if (result == NFD_CANCEL) {
		return {};
	}
	if (result == NFD_ERROR) {
		LOG_ERROR("NativeFileBrowserError: {}", NFD_GetError());
		return {};
	}

	std::optional<fs::path> path{ ToForwardSlashPath(outPath) };
	free(outPath);
	return path;
}

std::optional<std::vector<fs::path>> NativeFileBrowser::OpenFileMultiple(
	const ExtensionFilter& extensions, const fs::path& initialPath)
{
	nfdpathset_t pathSet;
	nfdresult_t result
		= NFD_OpenDialogMultiple(extensions.filter.c_str(), U8(initialPath.u8string().c_str()), &pathSet);

	if (result == NFD_CANCEL) {
		return {};
	}
	if (result == NFD_ERROR) {
		LOG_ERROR("NativeFileBrowserError: {}", NFD_GetError());
		return {};
	}
	std::vector<fs::path> files;

	size_t i;
	for (i = 0; i < NFD_PathSet_GetCount(&pathSet); ++i) {
		nfdchar_t* path = NFD_PathSet_GetPath(&pathSet, i);
		files.emplace_back(ToForwardSlashPath(path));
	}
	NFD_PathSet_Free(&pathSet);

	return files;
}

std::optional<fs::path> NativeFileBrowser::SelectFolder(const fs::path& initialPath)
{
	nfdchar_t* outPath = NULL;
	nfdresult_t result = NFD_PickFolder(U8(initialPath.u8string().c_str()), &outPath);

	if (result == NFD_CANCEL) {
		return {};
	}
	if (result == NFD_ERROR) {
		LOG_ERROR("NativeFileBrowserError: {}", NFD_GetError());
		return {};
	}
	std::optional<fs::path> path{ ToForwardSlashPath(outPath) };
	free(outPath);
	return path;
}

std::optional<fs::path> NativeFileBrowser::SaveFile(const ExtensionFilter& extensions, const fs::path& initialPath)
{
	nfdchar_t* outPath = NULL;
	nfdresult_t result = NFD_SaveDialog(extensions.filter.c_str(), U8(initialPath.u8string().c_str()), &outPath);

	if (result == NFD_CANCEL) {
		return {};
	}
	if (result == NFD_ERROR) {
		LOG_ERROR("NativeFileBrowserError: {}", NFD_GetError());
		return {};
	}
	std::optional<fs::path> path{ ToForwardSlashPath(outPath) };
	free(outPath);
	return path;
}
} // namespace ed
