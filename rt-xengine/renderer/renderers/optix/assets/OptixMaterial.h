#ifndef OPTIXMATERIAL_H
#define OPTIXMATERIAL_H

#include "renderer/renderers/optix/OptixAsset.h"

namespace Renderer::Optix
{

	class OptixMaterial : public OptixAsset
	{

		optix::Material m_handle;

		// RGB: Albedo A: Opacity
		std::shared_ptr<OptixTexture> m_textSurfaceAlbedo;
		// RGB: Emission A: Ambient Occlusion
		std::shared_ptr<OptixTexture> m_textSurfaceEmission;
		// R: Reflectivity G: Roughness B: Metallic A: Translucency
		std::shared_ptr<OptixTexture> m_textSurfaceSpecularParameters;
		// RGB: Normal A: Height
		std::shared_ptr<OptixTexture> m_textSurfaceBump;

		bool m_hasNormalMap;

		std::shared_ptr<OptixProgram> m_closestHitProgram;
		std::shared_ptr<OptixProgram> m_anyHitProgram;
		
		std::string m_closestHitProgramName;
		std::string m_anyHitProgramName;

	public:
		OptixMaterial(OptixRendererBase* renderer);
		~OptixMaterial() = default;

		bool Load(Assets::XMaterial* data, uint32 closestHitRayType, Assets::StringFile* closestHitProgramSource,
			std::string closestHitProgramName, uint32 anyHitRayType, Assets::StringFile* anyHitProgramSource,
			std::string anyHitProgramName);

		optix::Material GetOptixHandle() const { return m_handle; }

		void ToString(std::ostream& os) const override { os << "asset-type: OptixMaterial, name: " << m_associatedDescription << ", ch: " << m_closestHitProgramName << ", ah: " << m_anyHitProgramName; }
	};

}

#endif // OPTIXMATERIAL_H
