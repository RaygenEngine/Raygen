#pragma once

#include "asset/pods/ModelPod.h"
#include "world/nodes/geometry/GeometryNode.h"

#define GLCheckError() ogl::GLCheckError_(__FILE__, __LINE__)

namespace ogl {

inline GLint GetGLWrapping(TextureWrapping textWrapping)
{
	switch (textWrapping) {
		case TextureWrapping::CLAMP_TO_EDGE: return GL_CLAMP_TO_EDGE;
		case TextureWrapping::MIRRORED_REPEAT: return GL_MIRRORED_REPEAT;
		case TextureWrapping::REPEAT: return GL_REPEAT;
		default: return GL_INVALID_ENUM;
	}
}

inline GLint GetGLFiltering(TextureFiltering textFiltering)
{
	switch (textFiltering) {
		case TextureFiltering::NEAREST: return GL_NEAREST;
		case TextureFiltering::LINEAR: return GL_LINEAR;
		case TextureFiltering::NEAREST_MIPMAP_NEAREST: return GL_NEAREST_MIPMAP_NEAREST;
		case TextureFiltering::NEAREST_MIPMAP_LINEAR: return GL_NEAREST_MIPMAP_LINEAR;
		case TextureFiltering::LINEAR_MIPMAP_NEAREST: return GL_LINEAR_MIPMAP_NEAREST;
		case TextureFiltering::LINEAR_MIPMAP_LINEAR: return GL_LINEAR_MIPMAP_LINEAR;
		default: return GL_INVALID_ENUM;
	}
}

inline GLint GetGLGeometryMode(GeometryMode geomMode)
{
	switch (geomMode) {
		case GeometryMode::POINTS: return GL_POINTS;
		case GeometryMode::LINE: return GL_LINE;
		case GeometryMode::LINE_LOOP: return GL_LINE_LOOP;
		case GeometryMode::LINE_STRIP: return GL_LINE_STRIP;
		case GeometryMode::TRIANGLES: return GL_TRIANGLES;
		case GeometryMode::TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
		case GeometryMode::TRIANGLE_FAN: return GL_TRIANGLE_FAN;
		default: return GL_INVALID_ENUM;
	}
}

inline GLint GetGLTextureTarget(TextureTarget target)
{
	switch (target) {
		case TextureTarget::TEXTURE_2D: return GL_TEXTURE_2D;
		case TextureTarget::TEXTURE_CUBEMAP: return GL_TEXTURE_CUBE_MAP;
		default: LOG_ABORT("Texture format not yet supported");
	}
	return GL_INVALID_ENUM;
}

inline GLenum GLCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
			case GL_INVALID_ENUM: error = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE: error = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION: error = "INVALID_OPERATION"; break;
			case GL_STACK_OVERFLOW: error = "STACK_OVERFLOW"; break;
			case GL_STACK_UNDERFLOW: error = "STACK_UNDERFLOW"; break;
			case GL_OUT_OF_MEMORY: error = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		LOG_ERROR("{} | {} ( {} )", error, file, line);
	}
	return errorCode;
}


inline void DebugBoundVAO(std::string baseMessage)
{
	baseMessage = "Querying VAO state, model: baseMessage";
	int32 vab;
	int32 eabb;
	int32 eabbs;
	int32 mva;
	int32 isOn = 1;
	int32 vaabb;

	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vab);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &eabb);
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &eabbs);

	baseMessage += ", VAO: " + std::to_string(vab);
	baseMessage += ", IBO: " + std::to_string(eabb) + ", size=" + std::to_string(eabbs);

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &mva);
	for (int32 i = 0; i < mva; ++i) {
		glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &isOn);
		if (isOn) {
			glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vaabb);
			baseMessage += ", attribute#" + std::to_string(i) + ": VBO=" + std::to_string(vaabb);
		}
	}
	LOG_DEBUG(baseMessage);
}
} // namespace ogl
