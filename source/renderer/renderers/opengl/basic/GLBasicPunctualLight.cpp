#include "pch/pch.h"

#include "renderer/renderers/opengl/basic/GLBasicPunctualLight.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "asset/AssetManager.h"

#include <glm/gtc/type_ptr.hpp>

namespace ogl {
GLBasicPunctualLight::GLBasicPunctualLight(PunctualLightNode* node)
	: NodeObserver<PunctualLightNode, GLRendererBase>(node)
{
	depthMapShader = GetGLAssetManager(this)->GenerateFromPodPath<GLShader>(
		"/engine-data/glsl/general/DepthCubemap_AlphaMask_Linear.json");

	depthMapShader->StoreUniformLoc("m");
	depthMapShader->StoreUniformLoc("vps[0]");
	depthMapShader->StoreUniformLoc("vps[1]");
	depthMapShader->StoreUniformLoc("vps[2]");
	depthMapShader->StoreUniformLoc("vps[3]");
	depthMapShader->StoreUniformLoc("vps[4]");
	depthMapShader->StoreUniformLoc("vps[5]");
	depthMapShader->StoreUniformLoc("baseColorFactor");
	depthMapShader->StoreUniformLoc("baseColorTexcoordIndex");
	depthMapShader->StoreUniformLoc("alphaCutoff");
	depthMapShader->StoreUniformLoc("mask");
	depthMapShader->StoreUniformLoc("center");
	depthMapShader->StoreUniformLoc("far");

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
	if (node->CastsShadows()) {
		return;
	}

	glViewport(0, 0, node->GetShadowMapWidth(), node->GetShadowMapHeight());

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(depthMapShader->programId);

	depthMapShader->SendFloat("far", node->GetFar());
	depthMapShader->SendVec3("center", node->GetWorldTranslation());

	auto vps = node->GetViewProjectionMatrices();

	for (auto& geometry : geometries) {
		auto m = geometry->node->GetWorldMatrix();

		depthMapShader->SendMat4("m", m);
		depthMapShader->SendMat4("vps[0]", vps[0]);
		depthMapShader->SendMat4("vps[1]", vps[1]);
		depthMapShader->SendMat4("vps[2]", vps[2]);
		depthMapShader->SendMat4("vps[3]", vps[3]);
		depthMapShader->SendMat4("vps[4]", vps[4]);
		depthMapShader->SendMat4("vps[5]", vps[5]);

		for (auto& glMesh : geometry->glModel->meshes) {

			GLMaterial* glMaterial = glMesh.material;
			const MaterialPod* materialData = glMaterial->LockData();

			if (materialData->unlit)
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
