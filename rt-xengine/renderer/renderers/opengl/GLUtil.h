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
		case GU_INVALID: return GL_INVALID_ENUM;
		default:		 return GL_INVALID_ENUM;
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
			RT_XENGINE_LOG_DEBUG("{} | {} ( {} )", error, file, line);
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
		RT_XENGINE_LOG_DEBUG(baseMessage);
	}
}
