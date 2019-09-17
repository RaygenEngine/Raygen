#include "pch.h"

#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "renderer/renderers/opengl/test/GLTestGeometry.h"
#include "world/World.h"
#include "world/nodes/user/freeform/FreeformUserNode.h"
#include "world/nodes/geometry/GeometryNode.h"
#include "asset/AssetManager.h"
#include "system/Engine.h"
#include "renderer/renderers/opengl/assets/GLModel.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/GLAssetManager.h"

#include "glad/glad.h"

namespace OpenGL
{
	GLTestRenderer::~GLTestRenderer()
	{
		glDeleteFramebuffers(1, &m_fbo);
		glDeleteTextures(1, &m_outTexture);
		glDeleteRenderbuffers(1, &m_depthStencilRbo);
	}

	bool GLTestRenderer::InitScene(int32 width, int32 height)
	{
		auto am = Engine::GetAssetManager();

		// shaders
		auto shaderAsset = AssetManager::GetOrCreate<ShaderPod>("screen_quad.shader.json");
		m_screenQuadShader = GetGLAssetManager()->GetOrMakeFromUri<GLShader>(am->GetPodPath(shaderAsset));
		
		shaderAsset = AssetManager::GetOrCreate<ShaderPod>("test.shader.json");
		m_testShader = GetGLAssetManager()->GetOrMakeFromUri<GLShader>(am->GetPodPath(shaderAsset));
		
		auto& testShader = *m_testShader;
		testShader += "mvp";
		testShader += "m";
		testShader += "normalMatrix";
		testShader += "mode";
		testShader += "viewPos";
		testShader += "baseColorFactor";
		testShader += "emissiveFactor";
		testShader += "metallicFactor";
		testShader += "roughnessFactor";
		testShader += "normalScale";
		testShader += "occlusionStrength";
		testShader += "alphaMode";
		testShader += "alphaCutoff";
		testShader += "doubleSided";
		testShader += "baseColorSampler";
		testShader += "metallicRoughnessSampler";
		testShader += "emissiveSampler";
		testShader += "normalSampler";
		testShader += "occlusionSampler";

		// world data
		auto* user = Engine::GetWorld()->GetAvailableNodeSpecificSubType<FreeformUserNode>();

		// TODO: better way to check world requirements
		if(!user)
		{
			LOG_FATAL("Missing freeform user node!" );
			return false;
		}

		m_camera = user->GetCamera();

		for (auto* geometryNode : Engine::GetWorld()->GetNodeMap<GeometryNode>())
			m_geometryObservers.emplace_back(CreateObserver<GLTestRenderer, GLTestGeometry>(this, geometryNode));

		// fbo
		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		
		glGenTextures(1, &m_outTexture);
		glBindTexture(GL_TEXTURE_2D, m_outTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outTexture, 0);

		glGenRenderbuffers(1, &m_depthStencilRbo);
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthStencilRbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthStencilRbo);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			LOG_FATAL("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		return true;
	}

	void GLTestRenderer::WindowResize(int32 width, int32 height)
	{

		glViewport(0, 0, width, height);
	}

	void GLTestRenderer::Render()
	{
		// first pass
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		const auto backgroundColor = Engine::GetWorld()->GetRoot()->GetBackgroundColor();
		glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT /*| GL_STENCIL_BUFFER_BIT*/);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		auto& testShader = *m_testShader;

		glUseProgram(testShader.GetId());

		// global uniforms
		glUniform3fv(testShader["viewPos"], 1, glm::value_ptr(m_camera->GetWorldTranslation()));
		glUniform1i(testShader["mode"], m_previewMode);

		const auto vp = m_camera->GetProjectionMatrix() * m_camera->GetViewMatrix();
		// render geometry (non-instanced)
		for (auto& geometry : m_geometryObservers)
		{
			auto m = geometry->GetNode()->GetWorldMatrix();
			auto mvp = vp * m;

			glUniformMatrix4fv(testShader["mvp"], 1, GL_FALSE, glm::value_ptr(mvp));
			glUniformMatrix4fv(testShader["m"], 1, GL_FALSE, glm::value_ptr(m));
			glUniformMatrix3fv(testShader["normalMatrix"], 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(m)))));
			
			for (auto& glMesh : geometry->glModel->GetGLMeshes())
			{
				glBindVertexArray(glMesh.vao);

				GLMaterial* glMaterial = glMesh.material;
				PodHandle<MaterialPod> materialData = glMaterial->m_materialPod;

				glUniform4fv(testShader["baseColorFactor"], 1, glm::value_ptr(materialData->baseColorFactor));
				glUniform3fv(testShader["emissiveFactor"], 1, glm::value_ptr(materialData->emissiveFactor));
				glUniform1f(testShader["metallicFactor"], materialData->metallicFactor);
				glUniform1f(testShader["roughnessFactor"], materialData->roughnessFactor);
				glUniform1f(testShader["normalScale"], materialData->normalScale);
				glUniform1f(testShader["occlusionStrength"], materialData->occlusionStrength);
				glUniform1i(testShader["alphaMode"], materialData->alphaMode);
				glUniform1f(testShader["alphaCutoff"], materialData->alphaCutoff);
				glUniform1i(testShader["doubleSided"], materialData->doubleSided);
				
				glUniformHandleui64ARB(testShader["baseColorSampler"], glMaterial->baseColorTexture->GetBindlessId());
				glUniformHandleui64ARB(testShader["metallicRoughnessSampler"], glMaterial->metallicRoughnessTexture->GetBindlessId());
				glUniformHandleui64ARB(testShader["occlusionSampler"], glMaterial->occlusionTexture->GetBindlessId());
				glUniformHandleui64ARB(testShader["emissiveSampler"], glMaterial->emissiveTexture->GetBindlessId());
				glUniformHandleui64ARB(testShader["normalSampler"], glMaterial->normalTexture->GetBindlessId());

				materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
									
				glDrawElements(GL_TRIANGLES, glMesh.count, GL_UNSIGNED_INT, (GLvoid*)0);
			
				glBindVertexArray(0);
			}
		}

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glUseProgram(0);

		// second pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(m_screenQuadShader->GetId());

		glBindTexture(GL_TEXTURE_2D, m_outTexture);

		// big triangle trick, no vao
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindTexture(GL_TEXTURE_2D, 0);
		
		glUseProgram(0);
	}

	void GLTestRenderer::Update()
	{
		if (Engine::GetInput()->IsKeyPressed(XVirtualKey::K1))
		{
			m_previewMode = m_previewMode - 1 < 0 ? PT_COUNT - 1 : m_previewMode - 1;
			LOG_INFO("Preview Mode set to: {}({})", utl::SurfacePreviewTargetModeString(m_previewMode), m_previewMode);
		}
		else if (Engine::GetInput()->IsKeyPressed(XVirtualKey::K2))
		{
			++m_previewMode %= PT_COUNT;
			LOG_INFO("Preview Mode set to: {}({})", utl::SurfacePreviewTargetModeString(m_previewMode), m_previewMode);
		}
	}
}
