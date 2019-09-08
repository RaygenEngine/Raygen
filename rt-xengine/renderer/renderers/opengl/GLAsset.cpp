#include "pch.h"

#include "renderer/renderers/opengl/GLAsset.h"

void OpenGL::GLAsset::MarkLoaded()
{
	LOG_DEBUG("Loaded asset's data in memory, {}", this);
	m_loaded = true;
}
