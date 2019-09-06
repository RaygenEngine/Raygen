#include "pch.h"
#include "renderer/Renderer.h"
#include "system/Engine.h"

AssetManager* Renderer::GetAssetManager() const
{
	return GetEngine()->GetAssetManager();
}

bool Renderer::InitRendering(HWND assochWnd, HINSTANCE instance)
{
	LOG_FATAL("This renderer does not have a windows-based rendering functionality!");
	return false;
}
