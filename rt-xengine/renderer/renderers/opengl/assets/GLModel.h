#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "assets/model/Model.h"

#include "GLAD/glad.h"

namespace OpenGL
{
	class GLModel : public GLAsset
	{
	public:
		
		struct GLMaterial
		{
			// RGB: Albedo A: Opacity
			std::shared_ptr<GLTexture> baseColorTexture;
			// R: empty, G: Roughness, B: Metal, A: empty
			std::shared_ptr<GLTexture> metallicRoughnessTexture;
			std::shared_ptr<GLTexture> normalTexture;
			std::shared_ptr<GLTexture> occlusionTexture;
			std::shared_ptr<GLTexture> emissiveTexture;

			glm::vec4 baseColorFactor = { 1.f, 1.f, 1.f, 1.f };
			glm::vec3 emissiveFactor = { 0.f, 0.f, 0.f };
			float metallicFactor = 1.f;
			float roughnessFactor = 1.f;

			// scaledNormal = normalize((<sampled normal texture value> * 2.0 - 1.0) * vec3(<normal scale>, <normal scale>, 1.0))
			float normalScale = 1.f;
			// occludedColor = lerp(color, color * <sampled occlusion texture value>, <occlusion strength>)
			float occlusionStrength = 1.f;

			// When alphaMode is set to MASK the alphaCutoff property specifies the cutoff threshold. If the alpha value is greater than or equal
			// to the alphaCutoff value then it is rendered as fully opaque, otherwise, it is rendered as fully transparent. alphaCutoff value is ignored for other modes.
			// The alpha value is defined in the baseColorTexture for metallic-roughness material model.
			AlphaMode alphaMode = AM_OPAQUE;
			float alphaCutoff = 0.5f;

			// The doubleSided property specifies whether the material is double sided. When this value is false, back-face culling is enabled. When this value is true,
			// back-face culling is disabled and double sided lighting is enabled. The back-face must have its normals reversed before the lighting equation is evaluated.
			bool doubleSided = false;
		};
		
		struct GLMesh
		{
			GLuint vao = 0u;
			GLuint ebo = 0u;

			// TODO: may use single in the future
			GLuint positionsVBO = 0u;
			GLuint normalsVBO = 0u;
			GLuint tangentsVBO = 0u;
			GLuint bitangentsVBO = 0u;
			GLuint textCoords0VBO = 0u;
			GLuint textCoords1VBO = 0u;

			GLMaterial material;

			GLint geometryMode = 0u;

			uint64 count = 0u;
		};

	private:

		GLMaterial LoadGLMaterial(const Model::Material& data);
		GLMesh LoadGLMesh(const Model::GeometryGroup& data, GLenum usage);
		
	protected:
		GLenum m_usage;

		std::vector<GLMesh> m_meshes;

	public:
		GLModel(GLAssetManager* glAssetManager, const std::string& name)
			: GLAsset(glAssetManager, name),
			m_usage(GL_STATIC_DRAW) {}
		virtual ~GLModel()
		{
			for(auto& mesh : m_meshes)
			{
				glDeleteBuffers(1, &mesh.positionsVBO);
				glDeleteBuffers(1, &mesh.normalsVBO);
				glDeleteBuffers(1, &mesh.tangentsVBO);
				glDeleteBuffers(1, &mesh.bitangentsVBO);
				glDeleteBuffers(1, &mesh.textCoords0VBO);
				glDeleteBuffers(1, &mesh.textCoords1VBO);
				glDeleteBuffers(1, &mesh.ebo);

				glDeleteVertexArrays(1, &mesh.vao);
			}
		}

		bool Load(Model* data);

		std::vector<GLMesh>& GetGLMeshes() { return m_meshes; }

		void ToString(std::ostream& os) const override { os << "asset-type: GLModel, name: " << m_name; }
	};

}
