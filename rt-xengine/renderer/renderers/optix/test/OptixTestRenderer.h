#ifndef OPTIXTESTRENDERER_H
#define OPTIXTESTRENDERER_H

#include "renderer/renderers/optix/OptixRendererBase.h"

namespace Renderer::Optix
{
	struct OptixTestCamera;
	struct OptixTestGeometry;

	enum RAY_TYPE : uint
	{
		RT_RESULT = 0,
		RT_COUNT
	};

	enum ENTRY_POINT : uint
	{
		EP_PINHOLE = 0,
		EP_COUNT
	};

	class OptixTestRenderer : public OptixRendererBase
	{
		optix::Group m_optixRoot;

		std::shared_ptr<OptixTestCamera> m_cameraObserver;
		std::vector<std::shared_ptr<OptixTestGeometry>> m_geometryObservers;

		std::shared_ptr<OptixTexture> m_skyTexture;
		std::vector<std::shared_ptr<OptixInstancedModel>> m_instancedGeometries;

		World::CameraNode* m_camera;

		// uses SurfacePreviewTargetMode from TargetEnums.h
		int32 m_previewMode;
	public:
		OptixTestRenderer(System::Engine* context);
		~OptixTestRenderer() = default;

		bool InitScene(int32 width, int32 height) override;
		void WindowResize(int32 width, int32 height) override;
		void Render() override;

		MAKE_METADATA(OptixTestRenderer)

		void Update() override;
		;
	};
}

#endif // OPTIXTESTRENDERER_H
