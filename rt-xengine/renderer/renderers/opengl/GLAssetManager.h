#pragma once

#include "system/EngineObject.h"
#include "assets/DiskAssetManager.h"
#include "assets/GLCubeMap.h"
#include "assets/GLTexture.h"
#include "assets/GLShader.h"
#include "assets/GLModel.h"

namespace Renderer::OpenGL
{
	class GLAssetManager : public System::EngineObject
	{

		Assets::MultiKeyAssetCache<GLCubeMap, Assets::CubeMap*, GLint, bool> m_glCubeMaps;
		Assets::MultiKeyAssetCache<GLTexture, Assets::Texture*, GLint, GLint, GLint, GLint, GLint> m_glTextures;
		Assets::MultiKeyAssetCache<GLShader, Assets::StringFile*, Assets::StringFile*> m_glShaders;
		Assets::MultiKeyAssetCache<GLModel, Assets::Model*> m_glModels;
		//Assets::MultiKeyAssetCache<GLInstancedModel, Assets::Model*, World::TriangleModelInstancedGeometryNode*> m_glInstancedModels;

	public:
		GLAssetManager(EngineObject* pObject);

		std::shared_ptr<GLCubeMap> RequestGLCubeMap(Assets::CubeMap* cubeMap, GLint wrapFlag = GL_REPEAT, bool mipMapping = false);
		std::shared_ptr<GLTexture> RequestGLTexture(Assets::Texture* texture, GLint minFilter = GL_LINEAR, GLint magFilter = GL_LINEAR, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT, GLint wrapR = GL_REPEAT);
		std::shared_ptr<GLShader> RequestGLShader(Assets::StringFile* vertexFile, Assets::StringFile* fragmentFile);
		std::shared_ptr<GLModel> RequestGLModel(Assets::Model* model);
		// TODO: fix this one
		//std::shared_ptr<GLInstancedModel> RequestGLInstancedModel(World::TriangleModelInstancedGeometryNode* nodeInstancer);
		//

		void ToString(std::ostream& os) const override { os << "object-type: GLAssetManager, name: " << GetObjectId(); }
	};

}
