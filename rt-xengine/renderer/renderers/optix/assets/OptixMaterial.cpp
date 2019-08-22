#include "pch.h"

#include "OptixMaterial.h"
#include "OptixProgram.h"
#include "OptixTexture.h"


namespace Renderer::Optix
{
	OptixMaterial::OptixMaterial(OptixRendererBase* renderer)
		: OptixAsset(renderer), m_hasNormalMap(false)
	{
	}

	bool OptixMaterial::Load(Assets::XMaterial* data, uint32 closestHitRayType, Assets::StringFile* closestHitProgramSource,
		std::string closestHitProgramName, uint32 anyHitRayType, Assets::StringFile* anyHitProgramSource,
		std::string anyHitProgramName)
	{
		SetIdentificationFromAssociatedDiskAssetIdentification(data->GetLabel());

		m_closestHitProgramName = closestHitProgramName;
		m_anyHitProgramName = anyHitProgramName;

		m_textSurfaceAlbedo = GetRenderer()->RequestOptixTexture(data->GetMapSurfaceAlbedo());

		m_textSurfaceEmission = GetRenderer()->RequestOptixTexture(data->GetMapSurfaceEmission());

		m_textSurfaceSpecularParameters = GetRenderer()->RequestOptixTexture(data->GetMapSurfaceSpecularParameters());

		m_textSurfaceBump = GetRenderer()->RequestOptixTexture(data->GetMapSurfaceBump());
		m_hasNormalMap = data->GetMapNormal();

		m_handle = GetOptixContext()->createMaterial();

		m_closestHitProgram = GetRenderer()->RequestOptixProgram(closestHitProgramSource, closestHitProgramName);
		m_anyHitProgram = GetRenderer()->RequestOptixProgram(anyHitProgramSource, anyHitProgramName);


		m_handle->setClosestHitProgram(closestHitRayType, m_closestHitProgram->GetOptixHandle());
		m_handle->setAnyHitProgram(anyHitRayType, m_anyHitProgram->GetOptixHandle());


		// bindless textures
		m_handle["Albedo_mapId"]->setInt(m_textSurfaceAlbedo->GetOptixHandle()->getId());
		m_handle["Emission_mapId"]->setInt(m_textSurfaceEmission->GetOptixHandle()->getId());
		m_handle["SpecularParameters_mapId"]->setInt(m_textSurfaceSpecularParameters->GetOptixHandle()->getId());
		m_handle["Bump_mapId"]->setInt(m_textSurfaceBump->GetOptixHandle()->getId());
		m_handle["hasNormalMap"]->setInt(m_hasNormalMap);

		return true;
	}
}
