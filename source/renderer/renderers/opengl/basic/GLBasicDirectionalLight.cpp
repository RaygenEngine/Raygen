#include "pch/pch.h"

#include "renderer/renderers/opengl/basic/GLBasicDirectionalLight.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "renderer/renderers/opengl/GLPreviewer.h"
#include "asset/AssetManager.h"
#include "core/MathAux.h"

#include <glm/gtc/type_ptr.hpp>

namespace ogl {

GLBasicDirectionalLight::GLBasicDirectionalLight(DirectionalLightNode* node)
	: NodeObserver<DirectionalLightNode, GLRendererBase>(node)
{
	depthMapShader
		= GetGLAssetManager(this)->GenerateFromPodPath<GLShader>("/engine-data/glsl/general/DepthMap_AlphaMask.json");
	depthMapShader->StoreUniformLoc("mvp");
	depthMapShader->StoreUniformLoc("baseColorFactor");
	depthMapShader->StoreUniformLoc("baseColorTexcoordIndex");
	depthMapShader->StoreUniformLoc("alphaCutoff");
	depthMapShader->StoreUniformLoc("mask");

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, node->GetShadowMapWidth(), node->GetShadowMapHeight(), 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	GetGLPreviewer(this)->AddPreview(shadowMap, node->GetName());

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	CLOG_ABORT(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete!");
}

GLBasicDirectionalLight::~GLBasicDirectionalLight()
{
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &shadowMap);
}

void GLBasicDirectionalLight::RenderShadowMap(const std::vector<GLBasicGeometry*>& geometries)
{
	if (!node->CastsShadows()) {
		return;
	}

	glViewport(0, 0, node->GetShadowMapWidth(), node->GetShadowMapHeight());

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(depthMapShader->programId);

	auto vp = node->GetViewProjectionMatrix();

	for (auto& geometry : geometries) {
		// light frustum culling
		if (!node->GetFrustum().IntersectsAABB(geometry->node->GetAABB())) {
			continue;
		}

		auto m = geometry->node->GetWorldMatrix();
		auto mvp = vp * m;

		depthMapShader->SendMat4("mvp", mvp);

		for (auto& glMesh : geometry->glModel->meshes) {
			GLMaterial* glMaterial = glMesh.material;
			const MaterialPod* materialData = glMaterial->LockData();

			if (materialData->castsShadows)
				continue;

			glBindVertexArray(glMesh.vao);


			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, glMaterial->baseColorTexture->id);

			depthMapShader->SendFloat("alphaCutoff", materialData->alphaCutoff);
			depthMapShader->SendVec4("baseColorFactor", materialData->baseColorFactor);
			depthMapShader->SendInt("baseColorTexcoordIndex", materialData->baseColorTexCoordIndex);
			depthMapShader->SendInt("mask", materialData->alphaMode == MaterialPod::MASK ? GL_TRUE : GL_FALSE);

			materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

			glDrawElements(GL_TRIANGLES, glMesh.indicesCount, GL_UNSIGNED_INT, (GLvoid*)0);
		}
	}

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void GLBasicDirectionalLight::DirtyNodeUpdate(DirtyFlagset nodeDirtyFlagset)
{
	using DF = DirectionalLightNode::DF;

	if (nodeDirtyFlagset[DF::ShadowsTextSize]) {
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, node->GetShadowMapWidth(), node->GetShadowMapHeight(), 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
}
} // namespace ogl
