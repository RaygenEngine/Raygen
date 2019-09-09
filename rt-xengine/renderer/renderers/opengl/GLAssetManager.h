#pragma once

#include "GLAD/glad.h"
#include "assets/texture/CubeMap.h"
#include "assets/other/utf8/StringFile.h"
#include "assets/model/Model.h"
#include "assets/CachingAux.h"

namespace OpenGL
{
	class GLCubeMap;
	class GLTexture;
	class GLShader; 
	class GLModel;
		
	class GLAssetManager
	{
		CachingAux::MultiKeyAssetCache<GLCubeMap, CubeMap*, GLint, bool> m_glCubeMaps;
		CachingAux::MultiKeyAssetCache<GLTexture, Texture*, GLint, GLint, GLint, GLint, GLint> m_glTextures;
		CachingAux::MultiKeyAssetCache<GLShader, StringFile*, StringFile*> m_glShaders;
		CachingAux::MultiKeyAssetCache<GLModel, Model*> m_glModels;
		//Assets::MultiKeyAssetCache<GLInstancedModel, Assets::Model*, World::TriangleModelInstancedGeometryNode*> m_glInstancedModels;

	public:
		std::shared_ptr<GLCubeMap> RequestGLCubeMap(CubeMap* cubeMap, GLint wrapFlag = GL_REPEAT, bool mipMapping = false);
		std::shared_ptr<GLTexture> RequestGLTexture(Texture* texture, GLint minFilter = GL_LINEAR, GLint magFilter = GL_LINEAR, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT, GLint wrapR = GL_REPEAT);
		std::shared_ptr<GLShader> RequestGLShader(StringFile* vertexFile, StringFile* fragmentFile);
		std::shared_ptr<GLModel> RequestGLModel(Model* model);
		// TODO: fix this one
		//std::shared_ptr<GLInstancedModel> RequestGLInstancedModel(World::TriangleModelInstancedGeometryNode* nodeInstancer);
		//
	};

}
