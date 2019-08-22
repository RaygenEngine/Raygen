#include "pch.h"

#include "OptixVRPathTracerRenderer.h"
#include "OptixVRPathTracerGeometry.h"
#include "OptixVRPathTracerHead.h"
#include "OptixVRPathTracerLight.h"

#include "renderer/renderers/optix/OptixUtil.h"

#include "renderer/renderers/optix/assets/OptixProgram.h"
#include "renderer/renderers/optix/assets/OptixInstancedModel.h"
#include "renderer/renderers/optix/assets/OptixTexture.h"
#include "world/nodes/sky/SkyHDRNode.h"

namespace Renderer::Optix
{

	OptixVRPathTracerRenderer::OptixVRPathTracerRenderer(System::Engine* context)
		: OptixRendererBase(context), m_currentDisplayingBuffer(DB_DENOISED), m_maxBounces(1u), m_maxSpp(1u)
	{
	}

	bool OptixVRPathTracerRenderer::InitScene(int32 width, int32 height)
	{
		m_optixContext->setRayTypeCount(RT_COUNT);
		m_optixContext->setEntryPointCount(EP_COUNT);
		m_optixContext->setMaxTraceDepth(16u);
		m_optixContext->setMaxCallableProgramDepth(16u);

		m_optixContext["radiance_ray_type"]->setUint(RT_RADIANCE);
		m_optixContext["shadow_ray_type"]->setUint(RT_SHADOW);
		m_optixContext["scene_epsilon"]->setFloat(SCENE_EPSILON);

		auto vrPathTracerPtx = GetDiskAssetManager()->LoadFileAsset<Assets::StringFile>("VRPathTracer.ptx");

		// Ray generation programs
		m_vrPathTraceRayGenL = RequestOptixProgram(vrPathTracerPtx.get(), "pathtrace_vr_left_eye");
		m_vrPathTraceRayGenR = RequestOptixProgram(vrPathTracerPtx.get(), "pathtrace_vr_right_eye");
		m_optixContext->setRayGenerationProgram(EP_PATHTRACE_LEFT_EYE, m_vrPathTraceRayGenL->GetOptixHandle());
		m_optixContext->setRayGenerationProgram(EP_PATHTRACE_RIGHT_EYE, m_vrPathTraceRayGenR->GetOptixHandle());
		// Miss program
		m_vrPathTraceMiss = RequestOptixProgram(vrPathTracerPtx.get(), "miss");
		m_optixContext->setMissProgram(EP_PATHTRACE_LEFT_EYE, m_vrPathTraceMiss->GetOptixHandle());
		m_optixContext->setMissProgram(EP_PATHTRACE_RIGHT_EYE, m_vrPathTraceMiss->GetOptixHandle());
		// Exception program
		m_vrPathTraceException = RequestOptixProgram(vrPathTracerPtx.get(), "exception");
		m_optixContext->setExceptionProgram(EP_PATHTRACE_LEFT_EYE, m_vrPathTraceException->GetOptixHandle());
		m_optixContext->setExceptionProgram(EP_PATHTRACE_RIGHT_EYE, m_vrPathTraceException->GetOptixHandle());

		m_optixContext["bad_color"]->setFloat(1.0f, 0.0f, 1.0f);

		m_pathTracingBuffer = CreateOptixBuffer(m_optixContext, RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4, width, height, true);
		m_optixContext["output_buffer"]->set(m_pathTracingBuffer);

		m_optixRoot = m_optixContext->createGroup();

		auto* user = GetWorld()->GetAvailableNodeSpecificSubType<World::OculusUserNode>();

		RT_XENGINE_ASSERT_RETURN_FALSE(user, "Missing oculus user node!");

		m_headObserver = CreateObserver<OptixVRPathTracerRenderer, OptixVRPathTracerHead>(this, user->GetHead());

		auto* light0 = GetWorld()->GetAnyAvailableNode<World::LightNode>();
		m_lightObserver = CreateObserver<OptixVRPathTracerRenderer, OptixVRPathTracerLight>(this, light0);

		bool hasDynamicType = false;
		for (auto* geometryNode : GetWorld()->GetNodeMap<World::TriangleModelInstancedGeometryNode>())
		{
			m_instancedGeometries.push_back(RequestOptixInstancedModel(RT_RADIANCE, vrPathTracerPtx.get(),
				"surface_shading", RT_SHADOW, vrPathTracerPtx.get(), "any_hit_shadow",
				geometryNode));
			hasDynamicType = geometryNode->GetModel()->GetType() == Assets::GT_DYNAMIC;
		}

		for (auto& igeom : m_instancedGeometries)
		{
			for (auto& tran : igeom->GetInstancingTransforms())
				m_optixRoot->addChild(tran);
		}

		for (auto* geometryNode : GetWorld()->GetNodeMap<World::TriangleModelGeometryNode>())
		{
			m_geometryObservers.emplace_back(CreateObserver<OptixVRPathTracerRenderer, OptixVRPathTracerGeometry>(this, geometryNode));

			hasDynamicType = geometryNode->GetModel()->GetType() == Assets::GT_DYNAMIC;
		}

		for (auto& geomObs : m_geometryObservers)
		{
			m_optixRoot->addChild(geomObs->transform);
		}

		optix::Acceleration as = m_optixContext->createAcceleration(ACCEL(AS_TRBVH));
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

		m_inputAlbedoBuffer = CreateOptixBuffer(m_optixContext, RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4, width, height, true);
		m_optixContext["input_albedo_buffer"]->set(m_inputAlbedoBuffer);

		m_inputNormalBuffer = CreateOptixBuffer(m_optixContext, RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4, width, height, true);
		m_optixContext["input_normal_buffer"]->set(m_inputNormalBuffer);

		m_denoiserBuffer = CreateOptixBuffer(m_optixContext, RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4, width, height, true);

		m_depthBuffer = CreateOptixBuffer(m_optixContext, RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT, width, height, true);
		m_optixContext["depth_buffer"]->set(m_depthBuffer);

		m_outBuffer = m_denoiserBuffer;

		m_denoiserStage = m_optixContext->createBuiltinPostProcessingStage("DLDenoiser");

		m_denoiserStage->declareVariable("input_buffer")->set(m_pathTracingBuffer);
		m_denoiserStage->declareVariable("output_buffer")->set(m_denoiserBuffer);;
		m_denoiserStage->declareVariable("input_albedo_buffer")->set(m_inputAlbedoBuffer);
		m_denoiserStage->declareVariable("input_normal_buffer")->set(m_inputNormalBuffer);
		//m_denoiserStage->declareVariable("maxmem")->setFloat(8000000000);
		//m_denoiserStage->declareVariable("hdr")->setUint(1);

		m_commandList = m_optixContext->createCommandList();

		m_commandList->appendLaunch(EP_PATHTRACE_LEFT_EYE, width / 2, height);
		m_commandList->appendLaunch(EP_PATHTRACE_RIGHT_EYE, width / 2, height);
		m_commandList->appendPostprocessingStage(m_denoiserStage, width, height);
		m_commandList->finalize();

		return true;
	}

	void OptixVRPathTracerRenderer::Render()
	{
		m_optixContext["max_bounces"]->setUint(m_maxBounces);
		m_optixContext["max_spp"]->setUint(m_maxSpp);
		m_optixContext["sqrt_max_spp"]->setUint(uint32(sqrtf(m_maxSpp)));

		
		m_optixContext["forced_roughness"]->setFloat(m_forcedRoughness);
		m_optixContext["forced_reflectance"]->setFloat(m_forcedReflectance);
		m_optixContext["enable_sampling_mask"]->setInt(m_enableSamplingMask);
		m_optixContext["light_intensity"]->setFloat(m_lightIntensity);
		m_optixContext["forced_metal"]->setFloat(m_forcedMetal);

		m_optixContext["time_"]->setFloat(GetWorld()->GetWorldTime());
		// re-projection algorithm
		// launch left eye, prepare left side of output buffers (result, albedo, normal)

		// launch right eye, re-project information from left side to right side, calculate un-compatible areas
		// denoise using the output buffers

		//tm.Start();

		m_commandList->execute();
		// TODO : integrate Oculus SDK from the engine/renderer side
	/*	glBindTexture(GL_TEXTURE_2D, GetRenderTarget()->glOculusSingleTextureSymmetricalFov.glTexture);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_outBuffer->getGLBOId());
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GetRenderTarget()->glOculusSingleTextureSymmetricalFov.width, GetRenderTarget()->glOculusSingleTextureSymmetricalFov.height, GL_RGBA, GL_FLOAT,
			nullptr);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindTexture(GL_TEXTURE_2D, GetRenderTarget()->glOculusSingleTextureSymmetricalFov.glDepth);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_depthBuffer->getGLBOId());
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GetRenderTarget()->glOculusSingleTextureSymmetricalFov.width, GetRenderTarget()->glOculusSingleTextureSymmetricalFov.height, GL_DEPTH_COMPONENT32F, GL_FLOAT,
			nullptr);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);*/

		//tm.Stop();
		//tm.Print("nn");
	}

	//void OptixVRPathTracerRenderer::OnUpdate(const CTickEvent& event)
	//{
	//	//auto& keyboard = GetInput().GetKeyboard();

	//	//if (keyboard.IsKeyPressed(XVK_m))
	//	//{
	//	//	++m_currentDisplayingBuffer %= DB_COUNT;

	//	//	switch (m_currentDisplayingBuffer)
	//	//	{
	//	//	case DB_DENOISED:
	//	//		RT_XENGINE_LOG_INFO("Switched to denoiser buffer output");
	//	//		m_outBuffer = m_denoiserBuffer;
	//	//		break;

	//	//	case DB_PATHTRACING:
	//	//		RT_XENGINE_LOG_INFO("Switched to pathtracing buffer output");
	//	//		m_outBuffer = m_pathTracingBuffer;
	//	//		break;

	//	//	case DB_ALBEDO:
	//	//		RT_XENGINE_LOG_INFO("Switched to albedo buffer output");
	//	//		m_outBuffer = m_inputAlbedoBuffer;
	//	//		break;

	//	//	case DB_NORMAL:
	//	//		RT_XENGINE_LOG_INFO("Switched to normal buffer output");
	//	//		m_outBuffer = m_inputNormalBuffer;
	//	//		break;

	//	//	case DB_DEPTH:
	//	//		RT_XENGINE_LOG_INFO("Switched to depth buffer output");
	//	//		m_outBuffer = m_depthBuffer;
	//	//		break;
	//	//	}
	//	//}

	//	//if (keyboard.IsKeyPressed(XVK_KP_PLUS))
	//	//{
	//	//	++m_maxSpp;
	//	//	RT_XENGINE_LOG_INFO("Increased max samples per pixel to {}", m_maxSpp);
	//	//}
	//	//
	//	//if (keyboard.IsKeyPressed(XVK_KP_MINUS))
	//	//{
	//	//	m_maxSpp = m_maxSpp - 1 == 0 ? 1 : m_maxSpp - 1;
	//	//	RT_XENGINE_LOG_INFO("Reduced max samples per pixel to {}", m_maxSpp);
	//	//}

	//	//if (keyboard.IsKeyPressed(XVK_KP_MULTIPLY))
	//	//{
	//	//	++m_maxBounces;
	//	//	RT_XENGINE_LOG_INFO("Increased max depth to {}", m_maxBounces);
	//	//}

	//	//if (keyboard.IsKeyPressed(XVK_KP_DIVIDE))
	//	//{
	//	//	m_maxBounces = m_maxBounces - 1 <= 0 ? 0 : m_maxBounces - 1;
	//	//	RT_XENGINE_LOG_INFO("Reduced max depth to {}", m_maxBounces);
	//	//}

	//	//if (keyboard.IsKeyPressed(XVK_KP_4))
	//	//{
	//	//	m_forcedMetal = m_forcedMetal - 0.01 < 0 ? 0 : m_forcedMetal - 0.01;
	//	//	RT_XENGINE_LOG_INFO("Reduced metal to {}", m_forcedMetal);
	//	//}

	//	//if (keyboard.IsKeyPressed(XVK_KP_5))
	//	//{
	//	//	m_forcedMetal = m_forcedMetal + 0.01 > 1 ? 1 : m_forcedMetal + 0.01;
	//	//	RT_XENGINE_LOG_INFO("Increased metal to {}", m_forcedMetal);
	//	//}

	//	//if (keyboard.IsKeyPressed(XVK_KP_7))
	//	//{
	//	//	m_forcedRoughness = m_forcedRoughness - 0.01 < 0 ? 0 : m_forcedRoughness - 0.01;
	//	//	RT_XENGINE_LOG_INFO("Reduced roughness to {}", m_forcedRoughness);
	//	//}

	//	//if (keyboard.IsKeyPressed(XVK_KP_8))
	//	//{
	//	//	m_forcedRoughness = m_forcedRoughness + 0.01 > 1 ? 1 : m_forcedRoughness + 0.01;
	//	//	RT_XENGINE_LOG_INFO("Increased roughness to {}", m_forcedRoughness);
	//	//}

	//	//if (keyboard.IsKeyPressed(XVK_j))
	//	//{
	//	//	m_forcedReflectance = m_forcedReflectance - 0.01 < 0 ? 0 : m_forcedReflectance - 0.01;
	//	//	RT_XENGINE_LOG_INFO("Reduced reflectance to {}", m_forcedReflectance);
	//	//}

	//	//if (keyboard.IsKeyPressed(XVK_k))
	//	//{
	//	//	m_forcedReflectance = m_forcedReflectance + 0.01 > 1 ? 1 : m_forcedReflectance + 0.01;
	//	//	RT_XENGINE_LOG_INFO("Increased reflectance to {}", m_forcedReflectance);
	//	//}

	//	//if (keyboard.IsKeyPressed(XVK_SPACEBAR))
	//	//{
	//	//	m_enableSamplingMask = !m_enableSamplingMask;
	//	//	RT_XENGINE_LOG_INFO("Enable sampling mask set to {}", m_enableSamplingMask);
	//	//}

	//	//if (keyboard.IsKeyPressed(XVK_F1))
	//	//{
	//	//	m_lightIntensity = m_lightIntensity - 100 < 0 ? 0 : m_lightIntensity - 100;
	//	//}

	//	//if (keyboard.IsKeyPressed(XVK_F2))
	//	//{
	//	//	m_lightIntensity+= 100;
	//	//}
	//}
}
