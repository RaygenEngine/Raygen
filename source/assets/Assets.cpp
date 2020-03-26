#include "pch.h"
#include "Assets.h"

void Assets::Init()
{
	AssetManager = new S_AssetManager();
	ImporterManager = new S_AssetImporterManager();
}

void Assets::Destroy()
{
	delete ImporterManager;
	delete AssetManager;
}
