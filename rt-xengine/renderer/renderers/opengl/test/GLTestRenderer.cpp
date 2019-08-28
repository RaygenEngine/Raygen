#include "pch.h"

#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "renderer/renderers/opengl/test/GLTestGeometry.h"
#include "world/World.h"
#include "world/nodes/user/freeform/FreeformUserNode.h"
#include "world/nodes/geometry/TriangleModelGeometryNode.h"
#include "world/nodes/sky/SkyHDRNode.h"
#include "assets/DiskAssetManager.h"


namespace Renderer::OpenGL
{
	GLTestRenderer::GLTestRenderer(System::Engine* context)
		: GLRendererBase(context), m_camera(nullptr), m_previewMode(PT_ALBEDO)
	{
	}

	bool GLTestRenderer::InitScene(int32 width, int32 height)
	{
		auto vertexSimpleShaderSource = GetDiskAssetManager()->LoadStringFileAsset("test/test.vert");
		auto vertexInstancedShaderSource = GetDiskAssetManager()->LoadStringFileAsset("test/test_instanced.vert");
		auto fragmentShaderSource = GetDiskAssetManager()->LoadStringFileAsset("test/test.frag");

		m_instancedShader = RequestGLShader(vertexInstancedShaderSource.get(), fragmentShaderSource.get());
		m_instancedShader->SetUniformLocation("albedoSampler");
		m_instancedShader->SetUniformLocation("emissionSampler");
		m_instancedShader->SetUniformLocation("specularParametersSampler");
		m_instancedShader->SetUniformLocation("bumpSampler");
		m_instancedShader->SetUniformLocation("skyHDRSampler");
		m_instancedShader->SetUniformLocation("depthSampler");
		m_instancedShader->SetUniformLocation("vp");
		m_instancedShader->SetUniformLocation("viewPos");
		m_instancedShader->SetUniformLocation("mode");

		m_nonInstancedShader = RequestGLShader(vertexSimpleShaderSource.get(), fragmentShaderSource.get());
		m_nonInstancedShader->SetUniformLocation("albedoSampler");
		m_nonInstancedShader->SetUniformLocation("emissionSampler");
		m_nonInstancedShader->SetUniformLocation("specularParametersSampler");
		m_nonInstancedShader->SetUniformLocation("bumpSampler");
		m_nonInstancedShader->SetUniformLocation("skyHDRSampler");
		m_nonInstancedShader->SetUniformLocation("depthSampler");
		m_nonInstancedShader->SetUniformLocation("mvp");
		m_nonInstancedShader->SetUniformLocation("m");
		m_nonInstancedShader->SetUniformLocation("normal_matrix");
		m_nonInstancedShader->SetUniformLocation("viewPos");
		m_nonInstancedShader->SetUniformLocation("mode");

		auto* user = GetWorld()->GetAvailableNodeSpecificSubType<World::FreeformUserNode>();

		RT_XENGINE_ASSERT_RETURN_FALSE(user, "Missing freeform user node!");

		m_camera = user->GetCamera();

		// TODO: fix instanced geometry
		//for (auto* geometryNode : GetWorld()->GetNodeMap<World::TriangleModelInstancedGeometryNode>())
		//	m_instancedGeometries.emplace_back(RequestGLInstancedModel(geometryNode));

		for (auto* geometryNode : GetWorld()->GetNodeMap<World::TriangleModelGeometryNode>())
			m_geometryObservers.emplace_back(CreateObserver<GLTestRenderer, GLTestGeometry>(this, geometryNode));

		auto* sky = GetWorld()->GetAvailableNodeSpecificSubType<World::SkyHDRNode>();

		RT_XENGINE_ASSERT_RETURN_FALSE(sky, "Missing freeform user node!");

		m_skyTexture = RequestGLTexture(sky->GetSkyHDR());

		glViewport(0, 0, width, height);

		return true;
	}

	void GLTestRenderer::WindowResize(int32 width, int32 height)
	{
		glViewport(0, 0, width, height);
	}

	void GLTestRenderer::Render()
	{
		auto bgcl = GetWorld()->GetBackgroundColor();
		glClearColor(bgcl.r, bgcl.g, bgcl.b, 1.0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(m_instancedShader->GetGLHandle());

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);

		glUniform1i(m_instancedShader->GetUniformLocation("albedoSampler"), 0);
		glUniform1i(m_instancedShader->GetUniformLocation("emissionSampler"), 1);
		glUniform1i(m_instancedShader->GetUniformLocation("specularParametersSampler"), 2);
		glUniform1i(m_instancedShader->GetUniformLocation("bumpSampler"), 3);
		glUniform1i(m_instancedShader->GetUniformLocation("skyHDRSampler"), 4);
		glUniform1i(m_instancedShader->GetUniformLocation("depthSampler"), 5);

		glUniformMatrix4fv(m_instancedShader->GetUniformLocation("vp"), 1, GL_FALSE, &m_camera->GetViewProjectionMatrix()[0][0]);

		glUniform3fv(m_instancedShader->GetUniformLocation("viewPos"), 1, &m_camera->GetWorldTranslation()[0]);
		glUniform1i(m_instancedShader->GetUniformLocation("mode"), m_previewMode);

		// render instanced geometry
		for (auto& glInstancedModel : m_instancedGeometries)
		{
			for (auto& rm : glInstancedModel->GetRenderMeshes())
			{
				glBindVertexArray(rm.vao);

				for (auto& gg : rm.mesh->GetGeometryGroups())
				{

	/*				glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, gg.material->GetTextureSurfaceAlbedo()->GetGLHandle());

					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, gg.material->GetTextureSurfaceEmission()->GetGLHandle());

					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, gg.material->GetTextureSurfaceSpecularParameters()->GetGLHandle());

					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, gg.material->GetTextureSurfaceBump()->GetGLHandle());*/

					glActiveTexture(GL_TEXTURE4);
					glBindTexture(GL_TEXTURE_2D, m_skyTexture->GetGLHandle());

					// draw every gg instanced
					glDrawElementsInstanced(GL_TRIANGLES, sizeof(glm::u32vec3) * gg.indicesCount,
						GL_UNSIGNED_INT, (GLvoid*)(sizeof(glm::u32vec3) * gg.indicesOffset), glInstancedModel->GetInstancesCount());
				}

				glActiveTexture(GL_TEXTURE0);

				glBindVertexArray(0);
			}
		}

		glUseProgram(m_nonInstancedShader->GetGLHandle());

		glUniform1i(m_nonInstancedShader->GetUniformLocation("albedoSampler"), 0);
		glUniform1i(m_nonInstancedShader->GetUniformLocation("emissionSampler"), 1);
		glUniform1i(m_nonInstancedShader->GetUniformLocation("specularParametersSampler"), 2);
		glUniform1i(m_nonInstancedShader->GetUniformLocation("bumpSampler"), 3);
		glUniform1i(m_nonInstancedShader->GetUniformLocation("skyHDRSampler"), 4);
		glUniform1i(m_nonInstancedShader->GetUniformLocation("depthSampler"), 5);

		glUniform3fv(m_nonInstancedShader->GetUniformLocation("viewPos"), 1, &m_camera->GetWorldTranslation()[0]);

		glUniform1i(m_nonInstancedShader->GetUniformLocation("mode"), m_previewMode);

		// render geometry (non-instanced)
		for (auto& geometry : m_geometryObservers)
		{
			auto m = geometry->GetNode()->GetWorldMatrix();
			auto mvp = m_camera->GetViewProjectionMatrix() * m;

			glUniformMatrix4fv(m_nonInstancedShader->GetUniformLocation("mvp"), 1, GL_FALSE, &mvp[0][0]);
			glUniformMatrix4fv(m_nonInstancedShader->GetUniformLocation("m"), 1, GL_FALSE, &m[0][0]);
			glUniformMatrix3fv(m_nonInstancedShader->GetUniformLocation("normal_matrix"), 1, GL_FALSE, &glm::transpose(glm::inverse(glm::mat3(m)))[0][0]);

			for (auto& rm : geometry->glModel->GetRenderMeshes())
			{
				glBindVertexArray(rm.vao);

	
				for (auto& gg : rm.mesh->GetGeometryGroups())
				{
					//glActiveTexture(GL_TEXTURE0);
					//glBindTexture(GL_TEXTURE_2D, gg.material->GetTextureSurfaceAlbedo()->GetGLHandle());

					//glActiveTexture(GL_TEXTURE1);
					//glBindTexture(GL_TEXTURE_2D, gg.material->GetTextureSurfaceEmission()->GetGLHandle());

					//glActiveTexture(GL_TEXTURE2);
					//glBindTexture(GL_TEXTURE_2D, gg.material->GetTextureSurfaceSpecularParameters()->GetGLHandle());

					//glActiveTexture(GL_TEXTURE3);
					//glBindTexture(GL_TEXTURE_2D, gg.material->GetTextureSurfaceBump()->GetGLHandle());

					glActiveTexture(GL_TEXTURE4);
					glBindTexture(GL_TEXTURE_2D, m_skyTexture->GetGLHandle());


					glDrawElements(GL_TRIANGLES, sizeof(glm::u32vec3) * gg.indicesCount,
						GL_UNSIGNED_INT, (GLvoid*)(sizeof(glm::u32vec3) * gg.indicesOffset));
				}
				glActiveTexture(GL_TEXTURE0);

				glBindVertexArray(0);
			}
		}

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glUseProgram(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		SwapBuffers();
	}

	void GLTestRenderer::Update()
	{
		if (GetInput().IsKeyPressed(XVK_1))
		{
			m_previewMode = m_previewMode - 1 < 0 ? PT_COUNT - 1 : m_previewMode - 1; // TODO
			RT_XENGINE_LOG_INFO("Preview Mode set to: {}"/*, SurfacePreviewTargetModeString(m_previewMode)*/);
		}
		else if (GetInput().IsKeyPressed(XVK_2))
		{
			++m_previewMode %= PT_COUNT;
			RT_XENGINE_LOG_INFO("Preview Mode set to: {}"/*, SurfacePreviewTargetModeString(m_previewMode)*/);
		}
	}
}
