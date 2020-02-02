#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"

#include <glad/glad.h>

namespace ogl {
struct GLShader;

class GLEditorRenderer : public GLRendererBase {

	// bounding boxes
	GLuint m_bbVao{ 0u };
	GLuint m_bbVbo{ 0u };

	GLShader* m_bBoxShader{ nullptr };


public:
	~GLEditorRenderer() override;

	void InitScene() override;

	// When calling this from children renderers, call it after you are done with your drawing, otherwise you may draw
	// on top of the editor
	void Render() override;

	virtual void RenderBoundingBoxes();

	bool SupportsEditor() override { return false; }
};
} // namespace ogl
