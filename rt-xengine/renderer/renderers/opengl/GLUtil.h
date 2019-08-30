#pragma once

#define GLCheckError() GLCheckError_(__FILE__, __LINE__) 

namespace Renderer::OpenGL
{
	inline GLenum GetGLUsage(GeometryUsage geomUsage)
	{
		switch (geomUsage)
		{
		case GU_DYNAMIC: return GL_DYNAMIC_DRAW;
		case GU_STATIC:  return GL_STATIC_DRAW;
		case GU_INVALID:
		default:		 return GL_INVALID_ENUM;
		}
	}

	inline GLint GetGLWrapping(TextureWrapping textWrapping)
	{
		switch (textWrapping)
		{
		case TW_CLAMP_TO_EDGE:	 return GL_CLAMP_TO_EDGE;
		case TW_MIRRORED_REPEAT: return GL_MIRRORED_REPEAT;
		case TW_REPEAT:			 return GL_REPEAT;
		case TW_INVALID:		 
		default:				 return GL_INVALID_ENUM;
		}
	}

	inline GLint GetGLFiltering(TextureFiltering textFiltering)
	{
		switch (textFiltering)
		{
		case TF_NEAREST:				return GL_NEAREST;
		case TF_LINEAR:					return GL_LINEAR;
		case TF_NEAREST_MIPMAP_NEAREST: return GL_NEAREST_MIPMAP_NEAREST;
		case TF_NEAREST_MIPMAP_LINEAR:  return GL_NEAREST_MIPMAP_LINEAR;
		case TF_LINEAR_MIPMAP_NEAREST:  return GL_LINEAR_MIPMAP_NEAREST;
		case TF_LINEAR_MIPMAP_LINEAR:   return GL_LINEAR_MIPMAP_LINEAR;
		case TF_INVALID:
		default:						return GL_INVALID_ENUM;
		}
	}

	inline GLint GetGLGeometryMode(GeometryMode geomMode)
	{
		switch (geomMode)
		{
		case GM_POINTS:	        return GL_POINTS;
		case GM_LINE:           return GL_LINE;
		case GM_LINE_LOOP:      return GL_LINE_LOOP;
		case GM_LINE_STRIP:     return GL_LINE_STRIP;
		case GM_TRIANGLES:      return GL_TRIANGLES;
		case GM_TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
		case GM_TRIANGLE_FAN:   return GL_TRIANGLE_FAN;
		case GM_INVALID:
		default:				return GL_INVALID_ENUM;
		}
	}
	
	inline GLenum GLCheckError_(const char *file, int line)
	{
		GLenum errorCode;
		while ((errorCode = glGetError()) != GL_NO_ERROR)
		{
			std::string error;
			switch (errorCode)
			{
			case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
			case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
			case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
			case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
			}
			RT_XENGINE_LOG_ERROR("{} | {} ( {} )", error, file, line);
		}
		return errorCode;
	}


	inline void DebugBoundVAO(std::string baseMessage)
	{
		baseMessage = "Querying VAO state, model: baseMessage";
		int vab, eabb, eabbs, mva, isOn(1), vaabb;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vab);
		glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &eabb);
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &eabbs);

		baseMessage += ", VAO: " + std::to_string(vab);
		baseMessage += ", IBO: " + std::to_string(eabb) + ", size=" + std::to_string(eabbs);

		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &mva);
		for (int32 i = 0; i < mva; ++i)
		{
			glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &isOn);
			if (isOn)
			{
				glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vaabb);
				baseMessage += ", attribute#" + std::to_string(i) + ": VBO=" + std::to_string(vaabb);

			}
		}
		RT_XENGINE_LOG_ERROR(baseMessage);
	}
}
