#include "pch.h"

#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "renderer/renderers/opengl/test/GLTestGeometry.h"
#include "world/World.h"
#include "world/nodes/user/freeform/FreeformUserNode.h"
#include "world/nodes/geometry/GeometryNode.h"
#include "asset/AssetManager.h"
#include "system/Engine.h"
#include "renderer/renderers/opengl/assets/GLCubeMap.h"
#include "renderer/renderers/opengl/assets/GLModel.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/GLAssetManager.h"

namespace OpenGL
{
	bool GLTestRenderer::InitScene(int32 width, int32 height)
	{
		auto am = Engine::GetAssetManager();
		
		const auto shaderAsset = AssetManager::GetOrCreate<ShaderPod>("test.shader.json");
		
		m_nonInstancedShader = GetGLAssetManager()->GetOrMakeFromUri<GLShader>(am->GetPodPath(shaderAsset));
		
		m_nonInstancedShader->SetUniformLocation("mvp");
		m_nonInstancedShader->SetUniformLocation("m");
		m_nonInstancedShader->SetUniformLocation("normalMatrix");
		m_nonInstancedShader->SetUniformLocation("mode");
		m_nonInstancedShader->SetUniformLocation("viewPos");
		m_nonInstancedShader->SetUniformLocation("baseColorFactor");
		m_nonInstancedShader->SetUniformLocation("emissiveFactor");
		m_nonInstancedShader->SetUniformLocation("metallicFactor");
		m_nonInstancedShader->SetUniformLocation("roughnessFactor");
		m_nonInstancedShader->SetUniformLocation("normalScale");
		m_nonInstancedShader->SetUniformLocation("occlusionStrength");
		m_nonInstancedShader->SetUniformLocation("alphaMode");
		m_nonInstancedShader->SetUniformLocation("alphaCutoff");
		m_nonInstancedShader->SetUniformLocation("doubleSided");
		m_nonInstancedShader->SetUniformLocation("baseColorSampler");
		m_nonInstancedShader->SetUniformLocation("occlusionMetallicRoughnessSampler");
		m_nonInstancedShader->SetUniformLocation("emissiveSampler");
		m_nonInstancedShader->SetUniformLocation("normalSampler");
		m_nonInstancedShader->SetUniformLocation("occlusionSampler");
	
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

		//auto* sky = GetWorld()->GetAvailableNodeSpecificSubType<World::SkyHDRNode>();

		//RT_XENGINE_ASSERT_RETURN_FALSE(sky, "Missing freeform user node!");

		//m_skyTexture = RequestGLTexture(sky->GetSkyHDR());

		return true;
	}

	void GLTestRenderer::WindowResize(int32 width, int32 height)
	{
		glViewport(0, 0, width, height);
	}

	void GLTestRenderer::Render()
	{
		auto bgcl = Engine::GetWorld()->GetRoot()->GetBackgroundColor();

		glm::mat4 vp = m_camera->GetProjectionMatrix() * m_camera->GetViewMatrix();

		glClearColor(bgcl.r, bgcl.g, bgcl.b, 1.0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glUseProgram(m_nonInstancedShader->GetGLHandle());


		// TODO: glm::value_ptr
		// global uniforms
		glUniform3fv(m_nonInstancedShader->GetUniformLocation("viewPos"), 1, &m_camera->GetWorldTranslation()[0]);
		glUniform1i(m_nonInstancedShader->GetUniformLocation("mode"), m_previewMode);

		// render geometry (non-instanced)
		for (auto& geometry : m_geometryObservers)
		{
			auto m = geometry->GetNode()->GetWorldMatrix();
			auto mvp = vp * m;

			GLModel* model = geometry->glModel;

			glUniformMatrix4fv(m_nonInstancedShader->GetUniformLocation("mvp"), 1, GL_FALSE, &mvp[0][0]);
			glUniformMatrix4fv(m_nonInstancedShader->GetUniformLocation("m"), 1, GL_FALSE, &m[0][0]);
			glUniformMatrix3fv(m_nonInstancedShader->GetUniformLocation("normalMatrix"), 1, GL_FALSE, &glm::transpose(glm::inverse(glm::mat3(m)))[0][0]);
			
			for (auto& glMesh : geometry->glModel->GetGLMeshes())
			{
				glBindVertexArray(glMesh.vao);

				GLMaterial& glMaterial = *glMesh.material;

				PodHandle<MaterialPod> materialData = glMaterial.GetMaterialAsset();
				
				glUniform4fv(m_nonInstancedShader->GetUniformLocation("baseColorFactor"), 1, glm::value_ptr(materialData->baseColorFactor));
				glUniform3fv(m_nonInstancedShader->GetUniformLocation("emissiveFactor"), 1, glm::value_ptr(materialData->emissiveFactor));
				glUniform1f(m_nonInstancedShader->GetUniformLocation("metallicFactor"), materialData->metallicFactor);
				glUniform1f(m_nonInstancedShader->GetUniformLocation("roughnessFactor"), materialData->roughnessFactor);
				glUniform1f(m_nonInstancedShader->GetUniformLocation("normalScale"), materialData->normalScale);
				glUniform1f(m_nonInstancedShader->GetUniformLocation("occlusionStrength"), materialData->occlusionStrength);
				glUniform1i(m_nonInstancedShader->GetUniformLocation("alphaMode"), materialData->alphaMode);
				glUniform1f(m_nonInstancedShader->GetUniformLocation("alphaCutoff"), materialData->alphaCutoff);
				glUniform1i(m_nonInstancedShader->GetUniformLocation("doubleSided"), materialData->doubleSided);
				
				glUniformHandleui64ARB(m_nonInstancedShader->GetUniformLocation("baseColorSampler"), glMaterial.GetBaseColorTexture()->GetGLBindlessHandle());
				glUniformHandleui64ARB(m_nonInstancedShader->GetUniformLocation("metallicRoughnessSampler"), glMaterial.GetMetallicRoughnessTexture()->GetGLBindlessHandle());
				glUniformHandleui64ARB(m_nonInstancedShader->GetUniformLocation("occlusionSampler"), glMaterial.GetOcclusionTexture()->GetGLBindlessHandle());
				glUniformHandleui64ARB(m_nonInstancedShader->GetUniformLocation("emissiveSampler"), glMaterial.GetEmissiveTexture()->GetGLBindlessHandle());

				// may not exist
				const auto normalText = glMaterial.GetNormalTexture();
				glUniformHandleui64ARB(m_nonInstancedShader->GetUniformLocation("normalSampler"), normalText ? normalText->GetGLBindlessHandle() : 0);

				materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
									
				glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(glMesh.count), GL_UNSIGNED_INT, (GLvoid*)0);
			
				glBindVertexArray(0);
			}
		}

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glUseProgram(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
