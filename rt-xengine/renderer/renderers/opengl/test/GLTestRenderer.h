#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"
#include "world/nodes/camera/CameraNode.h"

namespace Renderer::OpenGL
{
	struct GLTestGeometry;

	class GLTestRenderer :  public GLRendererBase
	{
	protected:
		std::shared_ptr<GLShader> m_instancedShader;
		std::shared_ptr<GLShader> m_nonInstancedShader;

		std::vector<std::shared_ptr<GLTestGeometry>> m_geometryObservers;

		std::vector<std::shared_ptr<GLInstancedModel>> m_instancedGeometries;

		std::shared_ptr<GLTexture> m_skyTexture;

		World::CameraNode* m_camera;

		int32 m_previewMode;

	public:
		GLTestRenderer(System::Engine* context);
		~GLTestRenderer() = default;

		bool InitScene(int32 width, int32 height) override;
		void WindowResize(int32 width, int32 height) override;
		void Render() override;

		MAKE_METADATA(GLTestRenderer)

		void Update() override;
	};
}
