#pragma once

#include "renderer/Renderer.h"
#include "assets/other/utf8/StringFile.h"
#include "assets/model/Material.h"
#include "assets/model/Mesh.h"
#include "assets/model/Model.h"
#include "assets/DiskAssetManager.h"
#include "world/nodes/geometry/TriangleModelInstancedGeometryNode.h"
#include "renderer/renderers/opengl/assets/GLModel.h"
#include "renderer/renderers/opengl/assets/GLCubeMap.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/assets/GLMaterial.h"
#include "renderer/renderers/opengl/assets/GLMesh.h"
#include "renderer/renderers/opengl/assets/GLInstancedModel.h"

#include "GLAD/glad.h"


namespace Renderer::OpenGL
{
	class GLRendererBase : public Renderer
	{
		HWND m_assochWnd;
		HDC m_hdc;
		HGLRC m_hglrc;
		
		Assets::MultiKeyAssetCache<GLCubeMap, Assets::CubeMap*, GLint, bool> m_glCubeMaps;
		Assets::MultiKeyAssetCache<GLTexture, Assets::Texture*, GLint, GLint, GLint, GLint, GLint> m_glTextures;
		Assets::MultiKeyAssetCache<GLShader, Assets::StringFile*, Assets::StringFile*> m_glShaders;
		Assets::MultiKeyAssetCache<GLModel, Assets::Model*> m_glModels;
		//Assets::MultiKeyAssetCache<GLInstancedModel, Assets::Model*, World::TriangleModelInstancedGeometryNode*> m_glInstancedModels;

	public:
		GLRendererBase(System::Engine* context);
		~GLRendererBase();

		bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
		void SwapBuffers() override;

		std::shared_ptr<GLCubeMap> RequestGLCubeMap(Assets::CubeMap* cubeMap, GLint wrapFlag = GL_REPEAT, bool mipMapping = false);
		std::shared_ptr<GLTexture> RequestGLTexture(Assets::Texture* texture, GLint minFilter = GL_LINEAR, GLint magFilter = GL_LINEAR, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT, GLint wrapR = GL_REPEAT);
		std::shared_ptr<GLShader> RequestGLShader(Assets::StringFile* vertexFile, Assets::StringFile* fragmentFile);
		std::shared_ptr<GLModel> RequestGLModel(Assets::Model* model);
		// TODO: fix this one
		//std::shared_ptr<GLInstancedModel> RequestGLInstancedModel(World::TriangleModelInstancedGeometryNode* nodeInstancer);
		//
		void ToString(std::ostream& os) const override { os << "renderer-type: GLRendererBase, id: " << GetObjectId(); }
	};

}
