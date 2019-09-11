#pragma once

#include "GLAD/glad.h"
#include "assets/texture/CubeMapAsset.h"
#include "assets/other/utf8/StringFileAsset.h"
#include "assets/model/ModelAsset.h"
#include "assets/CachingAux.h"

namespace OpenGL
{
	class GLCubeMap;
	class GLTexture;
	class GLShader; 
	class GLModel;
		
	class GLAssetManager
	{
		CachingAux::MultiKeyAssetCache<GLCubeMap, CubeMapAsset*, GLint, bool> m_glCubeMaps;
		CachingAux::MultiKeyAssetCache<GLTexture, TextureAsset*, GLint, GLint, GLint, GLint, GLint> m_glTextures;
		CachingAux::MultiKeyAssetCache<GLShader, StringFileAsset*, StringFileAsset*> m_glShaders;
		CachingAux::MultiKeyAssetCache<GLModel, ModelAsset*> m_glModels;
		//Assets::MultiKeyAssetCache<GLInstancedModel, Assets::ModelAsset*, World::TriangleModelInstancedGeometryNode*> m_glInstancedModels;

	public:
		std::shared_ptr<GLCubeMap> RequestGLCubeMap(CubeMapAsset* cubeMap, GLint wrapFlag = GL_REPEAT, bool mipMapping = false);
		std::shared_ptr<GLTexture> RequestGLTexture(TextureAsset* texture, GLint minFilter = GL_LINEAR, GLint magFilter = GL_LINEAR, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT, GLint wrapR = GL_REPEAT);
		std::shared_ptr<GLShader> RequestGLShader(StringFileAsset* vertexFile, StringFileAsset* fragmentFile);
		std::shared_ptr<GLModel> RequestGLModel(ModelAsset* model);
		// TODO: fix this one
		//std::shared_ptr<GLInstancedModel> RequestGLInstancedModel(World::TriangleModelInstancedGeometryNode* nodeInstancer);
		//
	};

}
