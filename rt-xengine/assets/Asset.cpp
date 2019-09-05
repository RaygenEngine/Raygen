#include "pch.h"

#include "assets/Asset.h"

void Asset::MarkLoaded()
{
	LOG_DEBUG("Loaded asset's data in memory, {}", this);
	m_loaded = true;
}

void Asset::Clear()
{

}

void Asset::Unload()
{
	if (m_loaded)
	{
		LOG_DEBUG("Unloaded asset's data from memory, {}", this);
		m_loaded = false;
		Clear();
	}
}

