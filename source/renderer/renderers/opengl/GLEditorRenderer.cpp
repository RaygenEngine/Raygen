#include "pch/pch.h"

#include "renderer/renderers/opengl/GLEditorRenderer.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "world/World.h"
#include "editor/imgui/ImguiImpl.h"
#include "editor/Editor.h"

#include <glad/glad.h>

namespace ogl {
void GLEditorRenderer::InitRendering(HWND assochWnd, HINSTANCE instance)
{
	GLRendererBase::InitRendering(assochWnd, instance);

	if (Engine::IsEditorEnabled()) {
		ImguiImpl::InitOpenGL();
	}
}

void GLEditorRenderer::InitScene()
{
	m_bBoxShader = GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/general/BBox.json");
	m_bBoxShader->StoreUniformLoc("vp");
	m_bBoxShader->StoreUniformLoc("color");

	// bounding boxes
	glCreateVertexArrays(1, &m_bbVao);

	glEnableVertexArrayAttrib(m_bbVao, 0);
	glVertexArrayAttribFormat(m_bbVao, 0, 3, GL_FLOAT, GL_FALSE, 0);

	glCreateBuffers(1, &m_bbVbo);
	// TODO: (48)
	glNamedBufferData(m_bbVbo, 48 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

	glVertexArrayAttribBinding(m_bbVao, 0, 0);
	glVertexArrayVertexBuffer(m_bbVao, 0, m_bbVbo, 0, sizeof(float) * 3);
}

void GLEditorRenderer::Render()
{
	GLRendererBase::Render();

	if (Engine::IsEditorActive()) {
		ImguiImpl::RenderOpenGL();
	}
}

void GLEditorRenderer::RenderBoundingBoxes()
{
	Node* node = nullptr;

	switch (Editor::GetBBoxDrawing()) {
		case EditorBBoxDrawing::None: return;
		case EditorBBoxDrawing::SelectedNode:
			node = Editor::GetSelectedNode();
			if (!node) {
				return;
			}
			break;
		case EditorBBoxDrawing::AllNodes: break;
		default:;
	}

	glBindVertexArray(m_bbVao);

	// TODO: simplify and apply to other default renderers (during transition to local AABBs)

	const auto RenderBox = [&](math::AABB box, glm::vec4 color) {
		const GLfloat data[] = {
			box.min.x,
			box.min.y,
			box.min.z,

			box.max.x,
			box.min.y,
			box.min.z,

			box.max.x,
			box.max.y,
			box.min.z,

			box.min.x,
			box.max.y,
			box.min.z,
			//
			box.min.x,
			box.min.y,
			box.max.z,

			box.max.x,
			box.min.y,
			box.max.z,

			box.max.x,
			box.max.y,
			box.max.z,

			box.min.x,
			box.max.y,
			box.max.z,
			//
			box.min.x,
			box.min.y,
			box.min.z,

			box.min.x,
			box.min.y,
			box.max.z,

			box.max.x,
			box.min.y,
			box.min.z,

			box.max.x,
			box.min.y,
			box.max.z,
			//
			box.max.x,
			box.max.y,
			box.min.z,

			box.max.x,
			box.max.y,
			box.max.z,

			box.min.x,
			box.max.y,
			box.min.z,

			box.min.x,
			box.max.y,
			box.max.z,
		};

		glNamedBufferSubData(m_bbVbo, 0, 48 * sizeof(float), data);

		glUseProgram(m_bBoxShader->programId);
		const auto vp = m_activeCamera->GetViewProjectionMatrix();
		m_bBoxShader->SendMat4("vp", vp);
		m_bBoxShader->SendVec4("color", color);

		// TODO: with fewer draw calls
		glDrawArrays(GL_LINE_LOOP, 0, 4);
		glDrawArrays(GL_LINE_LOOP, 4, 4);
		glDrawArrays(GL_LINES, 8, 8);
	};

	for (auto& observer : GetObservers()) {
		if (!node || node == observer->baseNode) {
			RenderBox(observer->baseNode->GetAABB(), { 1, 1, 1, 1 });
		}
	}

	glDisable(GL_DEPTH_TEST);
}

GLEditorRenderer::~GLEditorRenderer()
{
	glDeleteVertexArrays(1, &m_bbVao);
	glDeleteBuffers(1, &m_bbVbo);

	if (Engine::IsEditorEnabled()) {
		ImguiImpl::CleanupOpenGL();
	}
}

} // namespace ogl
