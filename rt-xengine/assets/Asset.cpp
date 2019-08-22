#include "pch.h"
#include "Asset.h"

namespace Assets
{
	Asset::Asset(System::EngineObject* pObject)
		: EngineObject(pObject),
		m_loaded(false)
	{
	}

	void Asset::MarkLoaded()
	{
		RT_XENGINE_LOG_DEBUG("Loaded asset's data in memory, {}", this);
		m_loaded = true;
	}

	void Asset::Clear()
	{

	}

	void Asset::Unload()
	{
		if (m_loaded)
		{
			RT_XENGINE_LOG_DEBUG("Unloaded asset's data from memory, {}", this);
			m_loaded = false;
			Clear();
		}
	}
}
