#include "pch.h"

#include "renderer/renderers/opengl/GLAssetManager.h"

namespace Renderer::OpenGL
{
	GLAssetManager::GLAssetManager(System::EngineObject* pObject)
		: EngineObject(pObject)
	{
	}

	std::shared_ptr<GLCubeMap> GLAssetManager::RequestGLCubeMap(Assets::CubeMap* cubeMap, GLint wrapFlag,
		bool mipMapping)
	{
		return Assets::LoadAssetAtMultiKeyCache<GLCubeMap>(m_glCubeMaps, this, cubeMap->GetName(), cubeMap, wrapFlag, mipMapping);
	}

	std::shared_ptr<GLTexture> GLAssetManager::RequestGLTexture(Assets::Texture* texture, GLint minFilter,
		GLint magFilter, GLint wrapS, GLint wrapT, GLint wrapR)
	{
		return Assets::LoadAssetAtMultiKeyCache<GLTexture>(m_glTextures, this, texture->GetName(), texture, minFilter, magFilter, wrapS, wrapT, wrapR);
	}

	std::shared_ptr<GLShader> GLAssetManager::RequestGLShader(Assets::StringFile* vertexFile, Assets::StringFile* fragmentFile)
	{
		const auto name = "vert> " + vertexFile->GetFileName() + "frag> " + fragmentFile->GetFileName();

		return Assets::LoadAssetAtMultiKeyCache<GLShader>(m_glShaders, this, name, vertexFile, fragmentFile);
	}

	std::shared_ptr<GLModel> GLAssetManager::RequestGLModel(Assets::Model* model)
	{
		return Assets::LoadAssetAtMultiKeyCache<GLModel>(m_glModels, this, model->GetName(), model);
	}

	// TODO: caching may not work correctly with instancing, check this out
	//std::shared_ptr<GLInstancedModel> GLRendererBase::RequestGLInstancedModel(World::TriangleModelInstancedGeometryNode* nodeInstancer)
	//{
	//	const auto name = nodeInstancer->GetModel()->GetName();

	//	return Assets::LoadAssetAtMultiKeyCache<GLInstancedModel>(m_glInstancedModels, this, name, nodeInstancer);
	//}
}
