#ifndef GLRENDERERBASE_H
#define GLRENDERERBASE_H

#include "renderer/Renderer.h"

#include "glad/glad.h"
#include "assets/other/utf8/StringFile.h"


namespace Renderer::OpenGL
{
	class GLTexture;
	class GLShader;
	class GLMaterial;
	class GLMesh;
	class GLModel;
	class GLInstancedModel;
	class GLCubeMap;

	class GLRendererBase : public Renderer
	{
		HWND m_assochWnd;
		HDC m_hdc;
		HGLRC m_hglrc;

		Assets::MultiKeyAssetCache<GLCubeMap, Assets::CubeMap*, GLint, bool> m_glCubeMaps;
		Assets::MultiKeyAssetCache<GLTexture, Assets::Texture*, GLint, bool> m_glTextures;
		Assets::MultiKeyAssetCache<GLShader, Assets::StringFile*, Assets::StringFile*> m_glShaders;
		Assets::MultiKeyAssetCache<GLMaterial, Assets::XMaterial*> m_glMaterials;
		Assets::MultiKeyAssetCache<GLMesh, Assets::XMesh*, GLenum> m_glMeshes;
		Assets::MultiKeyAssetCache<GLModel, Assets::XModel*> m_glModels;
		Assets::MultiKeyAssetCache<GLInstancedModel, Assets::XModel*, World::TriangleModelInstancedGeometryNode*> m_glInstancedModels;

	public:
		GLRendererBase(System::Engine* context);
		~GLRendererBase();

		bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
		
		// TODO: thanks OpenGL, where do I put this...?
		void SwapBuffers();

		std::shared_ptr<GLCubeMap> RequestGLCubeMap(Assets::CubeMap* cubeMap, GLint wrapFlag = GL_REPEAT, bool mipMapping = false);
		std::shared_ptr<GLTexture> RequestGLTexture(Assets::Texture* texture, GLint wrapFlag = GL_REPEAT, bool mipMapping = false);
		std::shared_ptr<GLShader> RequestGLShader(Assets::StringFile* vertexFile, Assets::StringFile* fragmentFile);
		std::shared_ptr<GLMaterial> RequestGLMaterial(Assets::XMaterial* material);
		std::shared_ptr<GLMesh> RequestGLMesh(Assets::XMesh* mesh, GLenum usage = GL_STATIC_DRAW);
		std::shared_ptr<GLModel> RequestGLModel(Assets::XModel* model);
		std::shared_ptr<GLInstancedModel> RequestGLInstancedModel(World::TriangleModelInstancedGeometryNode* nodeInstancer);
	};

}

#endif // GLRENDERERBASE_H
