#pragma once

#include "assets/other/utf8/StringFile.h"
#include "assets/model/Material.h"
#include "assets/model/Mesh.h"
#include "assets/model/Model.h"
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
		
		Core::MultiKeyAssetCache<GLCubeMap, Assets::CubeMap*, GLint, bool> m_glCubeMaps;
		Core::MultiKeyAssetCache<GLTexture, Assets::Texture*, GLint, bool> m_glTextures;
		Core::MultiKeyAssetCache<GLShader, Assets::StringFile*, Assets::StringFile*> m_glShaders;
		Core::MultiKeyAssetCache<GLMaterial, Assets::Material*> m_glMaterials;
		Core::MultiKeyAssetCache<GLMesh, Assets::Mesh*, GLenum> m_glMeshes;
		Core::MultiKeyAssetCache<GLModel, Assets::Model*> m_glModels;
		Core::MultiKeyAssetCache<GLInstancedModel, Assets::Model*, World::TriangleModelInstancedGeometryNode*> m_glInstancedModels;

	public:
		GLRendererBase(System::Engine* context);
		~GLRendererBase();

		bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
		
		// TODO: thanks OpenGL, where do I put this...?
		void SwapBuffers();

		std::shared_ptr<GLCubeMap> RequestGLCubeMap(Assets::CubeMap* cubeMap, GLint wrapFlag = GL_REPEAT, bool mipMapping = false);
		std::shared_ptr<GLTexture> RequestGLTexture(Assets::Texture* texture, GLint wrapFlag = GL_REPEAT, bool mipMapping = false);
		std::shared_ptr<GLShader> RequestGLShader(Assets::StringFile* vertexFile, Assets::StringFile* fragmentFile);
		std::shared_ptr<GLMaterial> RequestGLMaterial(Assets::Material* material);
		std::shared_ptr<GLMesh> RequestGLMesh(Assets::Mesh* mesh, GLenum usage = GL_STATIC_DRAW);
		std::shared_ptr<GLModel> RequestGLModel(Assets::Model* model);
		// TODO: fix this one
		//std::shared_ptr<GLInstancedModel> RequestGLInstancedModel(World::TriangleModelInstancedGeometryNode* nodeInstancer);
	};

}
