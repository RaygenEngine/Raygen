#pragma once

// TODO: re-write when there is time to
// WIP: may be buggy

#include "renderer/renderers/opengl/GLEditorRenderer.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"
#include "renderer/renderers/opengl/basic/GLBasicDirectionalLight.h"
#include "renderer/renderers/opengl/basic/GLBasicSpotLight.h"
#include "renderer/renderers/opengl/basic/GLBasicPunctualLight.h"

class CameraNode;

namespace ogl {
class GLPreviewerRenderer : public GLEditorRenderer {

	struct PreviewTarget {
		int32 target;
		int32 count;

		PreviewTarget(int32 target, int32 count)
			: target(target)
			, count(count)
		{
		}

		void Next();
		void Previous();
		virtual std::string CurrentToString() = 0;
	};

	struct MaterialPreviewTarget : PreviewTarget {
		enum : int32
		{
			ALBEDO = 0,
			METALLIC,
			ROUGHNESS,
			OCCLUSION,
			EMISSIVE,
			NORMALMAP,
			OPACITY,
			ALPHACUTOFF,
			ALPHAMODE,
			DOUBLESIDED,
			COUNT
		};

		MaterialPreviewTarget()
			: PreviewTarget(ALBEDO, COUNT)
		{
		}

		std::string CurrentToString() override;

	} m_materialPreviewTarget;


	struct GeometryPreviewTarget : PreviewTarget {
		enum : int32
		{
			POSITIONS = 0,
			NORMALS,
			TANGENTS,
			BITANGENTS,
			FINALNORMALS,
			TEXTCOORD0,
			TEXTCOORD1,
			COUNT
		};

		GeometryPreviewTarget()
			: PreviewTarget(POSITIONS, COUNT)
		{
		}

		std::string CurrentToString() override;

	} m_geometryPreviewTarget;

	struct TextCoordIndexPreviewTarget : PreviewTarget {
		enum : int32
		{
			ALBEDO = 0,
			METALLIC,
			ROUGHNESS,
			EMISSIVE,
			NORMAL,
			OCCLUSION,
			COUNT
		};

		TextCoordIndexPreviewTarget()
			: PreviewTarget(ALBEDO, COUNT)
		{
		}

		std::string CurrentToString() override;

	} m_textCoordPreviewTarget;

	struct FactorsPreviewTarget : PreviewTarget {
		enum : int32
		{
			ALBEDO = 0,
			METALLIC,
			ROUGHNESS,
			EMISSIVE,
			NORMAL,
			OCCLUSION,
			OPACITY,
			COUNT
		};

		FactorsPreviewTarget()
			: PreviewTarget(ALBEDO, COUNT)
		{
		}

		std::string CurrentToString() override;

	} m_factorsPreviewTarget;

	enum PreviewMode
	{
		PM_MATERIAL = 0,
		PM_FACTORS = 1,
		PM_GEOMETRY = 2,
		PM_TEXTCOORD = 3,
		PM_DEPTH,
		PM_LIGHTS
	} m_previewMode{ PM_MATERIAL };

	PreviewTarget* m_previewTarget{ &m_materialPreviewTarget };

	void GetPreviousLight();
	void GetNextLight();
	int32 m_lightPreviewIndex{ 0 };

protected:
	// shaders
	GLShader* m_previewShader{ nullptr };
	GLShader* m_cubemapPreviewShader{ nullptr };
	GLShader* m_textureQuadShader{ nullptr };

	// observers
	std::vector<GLBasicGeometry*> m_glGeometries;
	std::vector<GLBasicDirectionalLight*> m_glDirectionalLights;
	std::vector<GLBasicPunctualLight*> m_glPunctualLights;
	std::vector<GLBasicSpotLight*> m_glSpotLights;

	// raw nodes
	CameraNode* m_camera{ nullptr };

	// rendering
	GLuint m_previewFbo{ 0 };
	GLuint m_previewColorTexture{ 0 };
	GLuint m_previewDepthTexture{ 0 };

	// Init
	void InitObservers();
	void InitShaders();
	void InitRenderBuffers();

	// Render
	void RenderDirectionalLights();
	void RenderPunctualLights();
	void RenderSpotLights();
	void RenderPreviewBuffer();
	void RenderWindow();

	// Update
	void RecompileShaders();

public:
	~GLPreviewerRenderer();

	void InitScene() override;

	void Render() override;

	void Update() override;
}; // namespace ogl
} // namespace ogl
