#ifndef ASSET_H
#define ASSET_H

#include "system/EngineObject.h"

namespace Assets
{
	// asset works as a placeholder to the wrapped data
	class Asset : public System::EngineObject
	{
		bool m_loaded;
		std::string m_name;

	protected:

		Asset(System::EngineObject* pObject);
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

		void ToString(std::ostream& os) const override { os << "asset-type: Asset, id: " << GetObjectId(); }
	};

}

#endif // ASSET_H
