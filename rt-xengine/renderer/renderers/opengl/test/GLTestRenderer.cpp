#include "pch.h"

#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "renderer/renderers/opengl/test/GLTestGeometry.h"
#include "world/World.h"
#include "world/nodes/user/freeform/FreeformUserNode.h"
#include "world/nodes/geometry/TriangleModelGeometryNode.h"
#include "world/nodes/sky/SkyHDRNode.h"
#include "assets/DiskAssetManager.h"
#include "renderer/renderers/opengl/GLUtil.h"


namespace Renderer::OpenGL
{
	GLTestRenderer::GLTestRenderer(System::Engine* context)
		: GLRendererBase(context), m_camera(nullptr), m_previewMode(0)
	{
	}

	bool GLTestRenderer::InitScene(int32 width, int32 height)
	{
		auto vertexSimpleShaderSource = GetDiskAssetManager()->LoadStringFileAsset("test/test.vert");
		//auto vertexInstancedShaderSource = GetDiskAssetManager()->LoadStringFileAsset("test/test_instanced.vert");
		auto fragmentShaderSource = GetDiskAssetManager()->LoadStringFileAsset("test/test.frag");

		//m_instancedShader = RequestGLShader(vertexInstancedShaderSource.get(), fragmentShaderSource.get());
		//m_instancedShader->SetUniformLocation("albedoSampler");
		//m_instancedShader->SetUniformLocation("emissionSampler");
		//m_instancedShader->SetUniformLocation("specularParametersSampler");
		//m_instancedShader->SetUniformLocation("bumpSampler");
		//m_instancedShader->SetUniformLocation("skyHDRSampler");
		//m_instancedShader->SetUniformLocation("depthSampler");
		//m_instancedShader->SetUniformLocation("vp");
		//m_instancedShader->SetUniformLocation("viewPos");
		//m_instancedShader->SetUniformLocation("mode");

				// RGB: Albedo A: Opacity
		std::shared_ptr<GLTexture> m_baseColorTexture;
		// R: empty, G: Roughness, B: Metal, A: empty
		std::shared_ptr<GLTexture> m_metallicRoughnessTexture;
		std::shared_ptr<GLTexture> m_normalTexture;
		std::shared_ptr<GLTexture> m_occlusionTexture;
		std::shared_ptr<GLTexture> m_emissiveTexture;
		
		m_nonInstancedShader = RequestGLShader(vertexSimpleShaderSource.get(), fragmentShaderSource.get());
		m_nonInstancedShader->SetUniformLocation("baseColorSampler");
		m_nonInstancedShader->SetUniformLocation("metallicRoughnessSampler");
		m_nonInstancedShader->SetUniformLocation("emissiveSampler");
		//m_nonInstancedShader->SetUniformLocation("normalSampler");
		//m_nonInstancedShader->SetUniformLocation("occlusionSampler");
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
	
		auto* user = GetWorld()->GetAvailableNodeSpecificSubType<World::FreeformUserNode>();

		RT_XENGINE_ASSERT_RETURN_FALSE(user, "Missing freeform user node!");

		m_camera = user->GetCamera();

		// TODO: fix instanced geometry
		//for (auto* geometryNode : GetWorld()->GetNodeMap<World::TriangleModelInstancedGeometryNode>())
		//	m_instancedGeometries.emplace_back(RequestGLInstancedModel(geometryNode));

		for (auto* geometryNode : GetWorld()->GetNodeMap<World::TriangleModelGeometryNode>())
			m_geometryObservers.emplace_back(CreateObserver<GLTestRenderer, GLTestGeometry>(this, geometryNode));

		//auto* sky = GetWorld()->GetAvailableNodeSpecificSubType<World::SkyHDRNode>();

		//RT_XENGINE_ASSERT_RETURN_FALSE(sky, "Missing freeform user node!");

		//m_skyTexture = RequestGLTexture(sky->GetSkyHDR());

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

		glm::mat4 vp = m_camera->GetProjectionMatrix() * m_camera->GetViewMatrix();

		glClearColor(bgcl.r, bgcl.g, bgcl.b, 1.0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);

		//glUseProgram(m_instancedShader->GetGLHandle());
		
		//glUniform1i(m_instancedShader->GetUniformLocation("albedoSampler"), 0);
		//glUniform1i(m_instancedShader->GetUniformLocation("emissionSampler"), 1);
		//glUniform1i(m_instancedShader->GetUniformLocation("specularParametersSampler"), 2);
		//glUniform1i(m_instancedShader->GetUniformLocation("bumpSampler"), 3);
		//glUniform1i(m_instancedShader->GetUniformLocation("skyHDRSampler"), 4);
		//glUniform1i(m_instancedShader->GetUniformLocation("depthSampler"), 5);

		//glUniformMatrix4fv(m_instancedShader->GetUniformLocation("vp"), 1, GL_FALSE, &vp[0][0]);

		//glUniform3fv(m_instancedShader->GetUniformLocation("viewPos"), 1, &m_camera->GetWorldTranslation()[0]);
		//glUniform1i(m_instancedShader->GetUniformLocation("mode"), m_previewMode);

		// TODO render instanced geometry
	//	for (auto& glInstancedModel : m_instancedGeometries)
	//	{
	//		for (auto& rm : glInstancedModel->GetRenderMeshes())
	//		{
	//			//glBindVertexArray(rm.vao);

	//			//for (auto& gg : rm.mesh->GetGeometryGroups())
	//			{

	///*				glActiveTexture(GL_TEXTURE0);
	//				glBindTexture(GL_TEXTURE_2D, gg.material->GetTextureSurfaceAlbedo()->GetGLHandle());

	//				glActiveTexture(GL_TEXTURE1);
	//				glBindTexture(GL_TEXTURE_2D, gg.material->GetTextureSurfaceEmission()->GetGLHandle());

	//				glActiveTexture(GL_TEXTURE2);
	//				glBindTexture(GL_TEXTURE_2D, gg.material->GetTextureSurfaceSpecularParameters()->GetGLHandle());

	//				glActiveTexture(GL_TEXTURE3);
	//				glBindTexture(GL_TEXTURE_2D, gg.material->GetTextureSurfaceBump()->GetGLHandle());*/

	//				glActiveTexture(GL_TEXTURE4);
	//				glBindTexture(GL_TEXTURE_2D, m_skyTexture->GetGLHandle());

	//				// draw every gg instanced
	//				//glDrawElementsInstanced(GL_TRIANGLES, sizeof(glm::u32vec3) * gg.indicesCount,
	//				//	GL_UNSIGNED_INT, (GLvoid*)(sizeof(glm::u32vec3) * gg.indicesOffset), glInstancedModel->GetInstancesCount());
	//			}

	//			glActiveTexture(GL_TEXTURE0);

	//			glBindVertexArray(0);
	//		}
	//	}

		glUseProgram(m_nonInstancedShader->GetGLHandle());

		glUniform1i(m_nonInstancedShader->GetUniformLocation("baseColorSampler"), 0);
		glUniform1i(m_nonInstancedShader->GetUniformLocation("metallicRoughnessSampler"), 1);
		glUniform1i(m_nonInstancedShader->GetUniformLocation("emissiveSampler"), 2);
		//glUniform1i(m_nonInstancedShader->GetUniformLocation("normalSampler"), 2);
		//glUniform1i(m_nonInstancedShader->GetUniformLocation("occlusionSampler"), 3);


		glUniform3fv(m_nonInstancedShader->GetUniformLocation("viewPos"), 1, &m_camera->GetWorldTranslation()[0]);

		glUniform1i(m_nonInstancedShader->GetUniformLocation("mode"), m_previewMode);

		// render geometry (non-instanced)
		for (auto& geometry : m_geometryObservers)
		{
			auto m = geometry->GetNode()->GetWorldMatrix();
			auto mvp = vp * m;

			glUniformMatrix4fv(m_nonInstancedShader->GetUniformLocation("mvp"), 1, GL_FALSE, &mvp[0][0]);
			glUniformMatrix4fv(m_nonInstancedShader->GetUniformLocation("m"), 1, GL_FALSE, &m[0][0]);
			glUniformMatrix3fv(m_nonInstancedShader->GetUniformLocation("normalMatrix"), 1, GL_FALSE, &glm::transpose(glm::inverse(glm::mat3(m)))[0][0]);

			for (auto& glMesh : geometry->glModel->GetGLMeshes())
			{
				glBindVertexArray(glMesh->GetVAO());

				auto& glMaterial = glMesh->GetMaterial();
				
				glUniform4fv(m_nonInstancedShader->GetUniformLocation("baseColorFactor"), 1, &glMaterial.GetBaseColorFactor()[0]);
				glUniform3fv(m_nonInstancedShader->GetUniformLocation("emissiveFactor"), 1, &glMaterial.GetEmissiveFactor()[0]);
				glUniform1f(m_nonInstancedShader->GetUniformLocation("metallicFactor"), glMaterial.GetMetallicFactor());
				glUniform1f(m_nonInstancedShader->GetUniformLocation("roughnessFactor"), glMaterial.GetRoughnessFactor());
				glUniform1f(m_nonInstancedShader->GetUniformLocation("normalScale"), glMaterial.GetNormalScale());
				glUniform1f(m_nonInstancedShader->GetUniformLocation("occlusionStrength"), glMaterial.GetOcclusionStrength());
				glUniform1i(m_nonInstancedShader->GetUniformLocation("alphaMode"), glMaterial.GetAlphaMode());
				glUniform1f(m_nonInstancedShader->GetUniformLocation("alphaCutoff"), glMaterial.GetAlphaCutoff());
				glUniform1f(m_nonInstancedShader->GetUniformLocation("doubleSided"), glMaterial.IsDoubleSided());
	
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, glMaterial.GetBaseColorTexture()->GetGLHandle());
	
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, glMaterial.GetMetallicRoughnessTexture()->GetGLHandle());

				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, glMaterial.GetEmissiveTexture()->GetGLHandle());

				//glActiveTexture(GL_TEXTURE3);
				//glBindTexture(GL_TEXTURE_2D, glMesh.GetMaterial().GetOcclusionTexture()->GetGLHandle());

				//glActiveTexture(GL_TEXTURE4);
				//glBindTexture(GL_TEXTURE_2D, glMesh.GetMaterial().GetEmissiveTexture()->GetGLHandle());

				glDrawElements(GL_TRIANGLES, glMesh->GetCount(), GL_UNSIGNED_INT, (GLvoid*)0);
			
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
		if (GetInput().IsKeyPressed(XVirtualKey::K1))
		{
			m_previewMode = m_previewMode - 1 < 0 ? PT_COUNT - 1 : m_previewMode - 1;
			RT_XENGINE_LOG_INFO("Preview Mode set to: {}({})", SurfacePreviewTargetModeString(m_previewMode), m_previewMode);
		}
		else if (GetInput().IsKeyPressed(XVirtualKey::K2))
		{
			++m_previewMode %= PT_COUNT;
			RT_XENGINE_LOG_INFO("Preview Mode set to: {}({})", SurfacePreviewTargetModeString(m_previewMode), m_previewMode);
		}
	}
}
