#include "pch.h"

#include "assets/FileAsset.h"

void FileAsset::MarkLoaded()
{
	LOG_DEBUG("Loaded asset's data in memory, {}", this);
	m_loaded = true;
}

void FileAsset::Clear()
{

}

void FileAsset::Unload()
{
	if (m_loaded)
	{
		LOG_DEBUG("Unloaded asset's data from memory, {}", this);
		m_loaded = false;
		Clear();
	}
}

