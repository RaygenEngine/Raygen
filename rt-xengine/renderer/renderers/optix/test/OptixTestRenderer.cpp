#include "pch.h"

#include "OptixTestRenderer.h"
#include "renderer/renderers/optix/OptixUtil.h"

#include "OptixTestGeometry.h"

#include "renderer/renderers/optix/assets/OptixProgram.h"
#include "renderer/renderers/optix/assets/OptixInstancedModel.h"

#include "renderer/renderers/optix/assets/OptixTexture.h"
#include "world/nodes/sky/SkyHDRNode.h"

namespace Renderer::Optix
{
	OptixTestRenderer::OptixTestRenderer(System::Engine* context)
		: OptixRendererBase(context), m_camera(nullptr), m_previewMode(PM_ALBEDO)
	{
	}

	bool OptixTestRenderer::InitScene(int32 width, int32 height)
	{
		RT_XENGINE_LOG_INFO("Preview Mode set to: {}", SurfacePreviewTargetModeString(m_previewMode));

		m_optixContext->setRayTypeCount(RT_COUNT);
		m_optixContext->setEntryPointCount(EP_COUNT);
		m_optixContext->setMaxTraceDepth(1u);
		m_optixContext->setMaxCallableProgramDepth(1u);
		m_optixContext["result_ray_type"]->setUint(RT_RESULT);
		m_optixContext["scene_epsilon"]->setFloat(SCENE_EPSILON);

		// avoid caching stuff that typically won't be requested again, don't bloat classes with members just for caching
		auto testPtx = GetDiskAssetManager()->LoadFileAsset<Assets::StringFile>("test.ptx");

		// Ray generation program
		m_optixContext->setRayGenerationProgram(EP_PINHOLE, m_optixContext->createProgramFromPTXString(testPtx.get()->GetFileData(), "pinhole_camera"));
		// Miss program
		m_optixContext->setMissProgram(EP_PINHOLE, m_optixContext->createProgramFromPTXString(testPtx.get()->GetFileData(), "miss"));
		// Exception program
		m_optixContext->setExceptionProgram(EP_PINHOLE, m_optixContext->createProgramFromPTXString(testPtx.get()->GetFileData(), "exception"));
		m_optixContext["bad_color"]->setFloat(1.0f, 0.0f, 1.0f);

		m_outBuffer = CreateOptixBuffer(m_optixContext, RT_BUFFER_OUTPUT, RT_FORMAT_UNSIGNED_BYTE4, width, height, true);
		m_optixContext["output_buffer"]->set(m_outBuffer);

		m_optixRoot = m_optixContext->createGroup();

		auto* user = GetWorld()->GetAvailableNodeSpecificSubType<World::FreeformUserNode>();

		RT_XENGINE_ASSERT_RETURN_FALSE(user, "Missing freeform user node!");

		m_camera = user->GetCamera();

		bool hasDynamicType = false;
		for (auto* geometryNode : GetWorld()->GetNodeMap<World::TriangleModelInstancedGeometryNode>())
		{
			m_instancedGeometries.push_back(RequestOptixInstancedModel(RT_RESULT, testPtx.get(),
				"closest_hit", RT_RESULT, testPtx.get(), "any_hit",
				geometryNode));
			hasDynamicType = geometryNode->GetModel()->GetType() == Assets::GT_DYNAMIC;
		}

		for (auto& geom : m_instancedGeometries)
		{
			for (auto& tran : geom->GetInstancingTransforms())
				m_optixRoot->addChild(tran);
		}

		for (auto* geometryNode : GetWorld()->GetNodeMap<World::TriangleModelGeometryNode>())
		{
			m_geometryObservers.emplace_back(CreateObserver<OptixTestRenderer, OptixTestGeometry>(this, geometryNode));

			hasDynamicType = geometryNode->GetModel()->GetType() == Assets::GT_DYNAMIC;
		}

		for (auto& geomObs : m_geometryObservers)
		{
			m_optixRoot->addChild(geomObs->transform);
		}

		auto as = m_optixContext->createAcceleration(ACCEL(AS_TRBVH));
		// if one of the models is dynamic, the top accel should be able to adjust
		if (!hasDynamicType)
			as->setProperty(ACCEL_PROP(ASP_REFIT), "1");
		m_optixRoot->setAcceleration(as);

		// skymap 
		m_skyTexture = RequestOptixTexture(GetWorld()->GetAvailableNodeSpecificSubType<World::SkyHDRNode>()->GetSkyHDR());
		m_optixContext["sky_mapId"]->setInt(m_skyTexture->GetOptixHandle()->getId());

		// context setup 
		m_optixContext["top_object"]->set(m_optixRoot);
		m_optixContext["top_shadower"]->set(m_optixRoot);

		m_optixContext["ambient_light_color"]->setFloat(GetWorld()->GetAmbientColor().x,
			GetWorld()->GetAmbientColor().y,
			GetWorld()->GetAmbientColor().z);
		m_optixContext["bg_color"]->setFloat(GetWorld()->GetBackgroundColor().x,
			GetWorld()->GetBackgroundColor().y,
			GetWorld()->GetBackgroundColor().z);

		std::string groupString;
		PrintGroupRecursively(m_optixRoot, groupString);
		RT_XENGINE_LOG_DEBUG("Pinhole test scene: \n{}", groupString);

		// OpenGL interop stuff
		return OptixRendererBase::InitScene(width, height);
	}

	void OptixTestRenderer::WindowResize(int32 width, int32 height)
	{
		ResizeOptixBuffer(m_outBuffer, width, height);

		// OpenGL interop stuff
		OptixRendererBase::WindowResize(width, height);
	}

	void OptixTestRenderer::Render()
	{
		auto eye = m_camera->GetWorldTranslation();
		glm::vec3 u;
		glm::vec3 v;
		glm::vec3 w;
		m_camera->GetTracingVariables(u, v, w);

		m_optixContext["eye"]->setFloat(eye.x, eye.y, eye.z);
		m_optixContext["U"]->setFloat(u.x, u.y, u.z);
		m_optixContext["V"]->setFloat(v.x, v.y, v.z);
		m_optixContext["W"]->setFloat(w.x, w.y, w.z);

		RTsize width, height;
		m_outBuffer->getSize(width, height);

		m_optixContext->launch(EP_PINHOLE, width, height);

		// OpenGL interop stuff
		OptixRendererBase::Render();
	}

	void OptixTestRenderer::Update()
	{
		if (GetInput().IsKeyPressed(XVK_1))
		{
			m_previewMode = m_previewMode - 1 < 0 ? PM_COUNT - 1 : m_previewMode - 1;
			m_optixContext["mode"]->setInt(m_previewMode);

			RT_XENGINE_LOG_INFO("Preview Mode set to: {}", SurfacePreviewTargetModeString(m_previewMode));
		}
		else if (GetInput().IsKeyPressed(XVK_2))
		{
			++m_previewMode %= PM_COUNT;
			m_optixContext["mode"]->setInt(m_previewMode);

			RT_XENGINE_LOG_INFO("Preview Mode set to: {}", SurfacePreviewTargetModeString(m_previewMode));
		}
	}
}
