#include "pch.h"
#include "Assets.h"

void Assets::Init()
{
	ImporterManager = new AssetImporterManager_();
	AssetManager = new AssetManager_();
}

void Assets::Destroy()
{
	delete AssetManager;
	delete ImporterManager;
}
