#pragma once

#include "system/EngineObject.h"

namespace Assets
{
	// asset works as a placeholder to the wrapped data
	class Asset : public System::EngineObject
	{
		bool m_loaded;
		
	protected:
		std::string m_name;

		Asset(EngineObject* pObject, const std::string& name);
		virtual ~Asset() = default;

		// clear asset's inner data
		// TODO: pure virtual, every asset must be able to clean itself (gpu or cpu)
		virtual void Clear();

	public:
		bool IsLoaded() const { return m_loaded; }

		// call this in case of successful loading
		void MarkLoaded();

		// clear only if loaded
		void Unload();

		std::string GetName() const { return m_name; }

		void ToString(std::ostream& os) const override { os << "object-type: Asset, name: " << m_name; }
	};

}
