#include "pch.h"

#include "renderer/renderers/opengl/GLAssetManager.h"
#include "renderer/renderers/opengl/assets/GLCubeMap.h"
#include "renderer/renderers/opengl/assets/GLModel.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"

namespace OpenGL
{
	std::shared_ptr<GLCubeMap> GLAssetManager::RequestGLCubeMap(CubeMap* cubeMap, GLint wrapFlag,
		bool mipMapping)
	{
		return CachingAux::LoadAssetAtMultiKeyCache<GLCubeMap>(m_glCubeMaps, {}, cubeMap, wrapFlag, mipMapping);
	}

	std::shared_ptr<GLTexture> GLAssetManager::RequestGLTexture(Texture* texture, GLint minFilter,
		GLint magFilter, GLint wrapS, GLint wrapT, GLint wrapR)
	{
		return CachingAux::LoadAssetAtMultiKeyCache<GLTexture>(m_glTextures, texture->GetFileName(), texture, minFilter, magFilter, wrapS, wrapT, wrapR);
	}

	std::shared_ptr<GLShader> GLAssetManager::RequestGLShader(StringFile* vertexFile, StringFile* fragmentFile)
	{
		const auto name = "vert> " + vertexFile->GetFileName() + "frag> " + fragmentFile->GetFileName();

		return CachingAux::LoadAssetAtMultiKeyCache<GLShader>(m_glShaders, name, vertexFile, fragmentFile);
	}

	std::shared_ptr<GLModel> GLAssetManager::RequestGLModel(Model* model)
	{
		return CachingAux::LoadAssetAtMultiKeyCache<GLModel>(m_glModels, model->GetFileName(), model);
	}

	// TODO: caching may not work correctly with instancing, check this out
	//std::shared_ptr<GLInstancedModel> GLRendererBase::RequestGLInstancedModel(World::TriangleModelInstancedGeometryNode* nodeInstancer)
	//{
	//	const auto name = nodeInstancer->GetModel()->GetName();

	//	return Assets::LoadAssetAtMultiKeyCache<GLInstancedModel>(m_glInstancedModels, this, name, nodeInstancer);
	//}
}
