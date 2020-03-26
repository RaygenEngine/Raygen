#include "pch.h"
#include "Assets.h"

void Assets::Init()
{
	ImporterManager = new S_AssetImporterManager();
	AssetManager = new S_AssetManager();
}

void Assets::Destroy()
{
	delete AssetManager;
	delete ImporterManager;
}
