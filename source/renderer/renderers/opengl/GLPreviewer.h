#pragma once

#include <glad/glad.h>

namespace ogl {
class GLAssetManager;
struct GLShader;

class GLPreviewer {

	// TODO: add channel preview flag and cubemap previewing
	// TODO: split preview buffers so that are easily accessible
	struct Preview {
		GLuint handle;
		std::string name;
		std::string description;

		Preview(GLuint handle, const std::string& name, const std::string& description);
	};

	std::vector<Preview> m_previews;
	size_t m_currentPreview{ 0 };

	GLShader* m_simpleShader{ nullptr };

public:
	void InitPreviewShaders(GLAssetManager* contextManager);

	void AddPreview(GLuint handle, const std::string& name, const std::string& description = "");
	// overwrites preview on back buffer
	void RenderPreview();
	// TODO: fix bugs and edge cases
	// searching for previous valid
	void PreviousPreview();
	// searching for next valid
	void NextPreview();
};
} // namespace ogl
