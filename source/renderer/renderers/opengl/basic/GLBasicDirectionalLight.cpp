#include "pch/pch.h"

#include "renderer/renderers/opengl/basic/GLBasicDirectionalLight.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "asset/AssetManager.h"

#include <glm/gtc/type_ptr.hpp>

namespace ogl {
GLBasicDirectionalLight::GLBasicDirectionalLight(DirectionalLightNode* node)
	: NodeObserver<DirectionalLightNode, GLRendererBase>(node)
{
	depthMapShader = GetGLAssetManager(this)->GenerateFromPodPath<GLShader>("/shaders/glsl/general/DepthMap.json");
	depthMapShader->AddUniform("mvp");
	depthMapShader->AddUniform("base_color_factor");
	depthMapShader->AddUniform("base_color_texcoord_index");
	depthMapShader->AddUniform("alpha_cutoff");
	depthMapShader->AddUniform("mask");

	glGenFramebuffers(1, &fbo);

	glGenTextures(1, &shadowMap);

	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, node->GetShadowMapWidth(), node->GetShadowMapHeight(), 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
}

GLBasicDirectionalLight::~GLBasicDirectionalLight()
{
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &shadowMap);
}

void GLBasicDirectionalLight::RenderShadowMap(const std::vector<GLBasicGeometry*>& geometries)
{
	glViewport(0, 0, node->GetShadowMapWidth(), node->GetShadowMapHeight());

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_DEPTH_BUFFER_BIT);

	if (!node->CastsShadows())
		return;

	glUseProgram(depthMapShader->id);

	auto vp = node->GetViewProjectionMatrix();

	for (auto& geometry : geometries) {
		auto m = geometry->node->GetWorldMatrix();
		auto mvp = vp * m;

		depthMapShader->UploadMat4("mvp", mvp);

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

void GLBasicDirectionalLight::DirtyNodeUpdate(DirtyFlagset nodeDirtyFlagset)
{
	using DF = DirectionalLightNode::DF;

	if (nodeDirtyFlagset[DF::ResizeShadows]) {
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, node->GetShadowMapWidth(), node->GetShadowMapHeight(), 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
}
} // namespace ogl
