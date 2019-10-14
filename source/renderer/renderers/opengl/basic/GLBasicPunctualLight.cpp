#include "pch/pch.h"

#include "renderer/renderers/opengl/basic/GLBasicPunctualLight.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "asset/AssetManager.h"
#include "renderer/renderers/opengl/GLUtil.h"

#include <glm/gtc/type_ptr.hpp>

namespace ogl {
GLBasicPunctualLight::GLBasicPunctualLight(PunctualLightNode* node)
	: NodeObserver<PunctualLightNode, GLRendererBase>(node)
{
	depthMapShader = GetGLAssetManager(this)->GenerateFromPodPath<GLShader>(
		"/shaders/glsl/general/DepthCubemap_AlphaMask_Linear.json");

	depthMapShader->AddUniform("m");
	depthMapShader->AddUniform("vps[0]");
	depthMapShader->AddUniform("vps[1]");
	depthMapShader->AddUniform("vps[2]");
	depthMapShader->AddUniform("vps[3]");
	depthMapShader->AddUniform("vps[4]");
	depthMapShader->AddUniform("vps[5]");
	depthMapShader->AddUniform("base_color_factor");
	depthMapShader->AddUniform("center");
	depthMapShader->AddUniform("far");
	depthMapShader->AddUniform("base_color_texcoord_index");
	depthMapShader->AddUniform("alpha_cutoff");
	depthMapShader->AddUniform("mask");

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &cubeShadowMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeShadowMap);
	for (auto i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, node->GetShadowMapWidth(),
			node->GetShadowMapHeight(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubeShadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	CLOG_ABORT(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE,
		"ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
}

GLBasicPunctualLight::~GLBasicPunctualLight()
{
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &cubeShadowMap);
}

void GLBasicPunctualLight::RenderShadowMap(const std::vector<GLBasicGeometry*>& geometries)
{
	glViewport(0, 0, node->GetShadowMapWidth(), node->GetShadowMapHeight());

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glClear(GL_DEPTH_BUFFER_BIT);

	if (!node->CastsShadows())
		return;

	glUseProgram(depthMapShader->programId);

	depthMapShader->UploadFloat("far", node->GetFar());
	depthMapShader->UploadVec3("center", node->GetWorldTranslation());

	auto vps = node->GetViewProjectionMatrices();

	for (auto& geometry : geometries) {
		auto m = geometry->node->GetWorldMatrix();

		depthMapShader->UploadMat4("m", m);
		depthMapShader->UploadMat4("vps[0]", vps[0]);
		depthMapShader->UploadMat4("vps[1]", vps[1]);
		depthMapShader->UploadMat4("vps[2]", vps[2]);
		depthMapShader->UploadMat4("vps[3]", vps[3]);
		depthMapShader->UploadMat4("vps[4]", vps[4]);
		depthMapShader->UploadMat4("vps[5]", vps[5]);

		for (auto& glMesh : geometry->glModel->meshes) {

			GLMaterial* glMaterial = glMesh.material;
			const MaterialPod* materialData = glMaterial->LockData();

			if (materialData->unlit)
				continue;

			glBindVertexArray(glMesh.vao);


			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, glMaterial->baseColorTexture->id);

			depthMapShader->UploadFloat("alpha_cutoff", materialData->alphaCutoff);
			depthMapShader->UploadVec4("base_color_factor", materialData->baseColorFactor);
			depthMapShader->UploadInt("base_color_texcoord_index", materialData->baseColorTexCoordIndex);
			depthMapShader->UploadInt("mask", materialData->alphaMode == MaterialPod::MASK ? GL_TRUE : GL_FALSE);

			materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

			glDrawElements(GL_TRIANGLES, glMesh.count, GL_UNSIGNED_INT, (GLvoid*)0);
		}
	}

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void GLBasicPunctualLight::DirtyNodeUpdate(DirtyFlagset nodeDirtyFlagset)
{
	using DF = PunctualLightNode::DF;

	if (nodeDirtyFlagset[DF::ShadowsTextSize]) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeShadowMap);
		for (auto i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, node->GetShadowMapWidth(),
				node->GetShadowMapHeight(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
}
} // namespace ogl
