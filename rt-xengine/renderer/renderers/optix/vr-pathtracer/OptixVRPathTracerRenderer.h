#ifndef OPTIXVRPATHTRACERRENDERER_H
#define OPTIXVRPATHTRACERRENDERER_H

#include "renderer/renderers/optix/OptixRendererBase.h"
#include <iostream>
#include <iomanip>

namespace Renderer::Optix
{
	struct OptixVRPathTracerGeometry;
	struct OptixVRPathTracerHead;
	struct OptixVRPathTracerLight;

	enum RAY_TYPE : uint
	{
		RT_RADIANCE = 0,
		RT_SHADOW,
		RT_COUNT
	};

	enum ENTRY_POINT : uint
	{
		EP_PATHTRACE_LEFT_EYE = 0,
		EP_PATHTRACE_RIGHT_EYE,
		EP_COUNT
	};

	enum DISPLAYING_BUFFER : uint32
	{
		DB_DENOISED = 0,
		DB_PATHTRACING,
		DB_ALBEDO,
		DB_NORMAL,
		DB_DEPTH,
		DB_COUNT
	};

	class OptixVRPathTracerRenderer : public OptixRendererBase
	{
		optix::Group m_optixRoot;

		optix::CommandList m_commandList;
		optix::PostprocessingStage m_denoiserStage;

		uint32 m_currentDisplayingBuffer;

		optix::Buffer m_outBuffer;

		optix::Buffer m_pathTracingBuffer;
		optix::Buffer m_denoiserBuffer;
		optix::Buffer m_inputAlbedoBuffer;
		optix::Buffer m_inputNormalBuffer;
		optix::Buffer m_depthBuffer;

		float m_forcedRoughness = 0.8f;
		float m_forcedMetal = 0.0f;
		float m_forcedReflectance = 0.5f;
		float m_lightIntensity = 100.0f;
		int m_enableSamplingMask = true;

		std::shared_ptr<OptixVRPathTracerHead> m_headObserver;
		std::shared_ptr<OptixVRPathTracerLight> m_lightObserver;
		std::vector<std::shared_ptr<OptixVRPathTracerGeometry>> m_geometryObservers;

		std::shared_ptr<OptixTexture> m_skyTexture;

		std::vector<std::shared_ptr<OptixInstancedModel>> m_instancedGeometries;

		std::shared_ptr<OptixProgram> m_vrPathTraceDiffuseClosestHit;
		std::shared_ptr<OptixProgram> m_vrPathTraceRayGenL;
		std::shared_ptr<OptixProgram> m_vrPathTraceRayGenR;
		std::shared_ptr<OptixProgram> m_vrPathTraceMiss;
		std::shared_ptr<OptixProgram> m_vrPathTraceException;

		uint32 m_maxBounces;
		uint32 m_maxSpp;

	public:
		OptixVRPathTracerRenderer(System::Engine* context);
		~OptixVRPathTracerRenderer() = default;

		bool InitScene(int32 width, int32 height) override;

		void Render() override;

		MAKE_METADATA(OptixVRPathTracerRenderer);
	};
}

#endif // OPTIXVRPATHTRACERRENDERER_H
