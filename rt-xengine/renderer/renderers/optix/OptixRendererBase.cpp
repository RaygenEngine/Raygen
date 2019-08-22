#include "pch.h"
#include "OptixRendererBase.h"


#include "assets/OptixTexture.h"
#include "assets/OptixProgram.h"
#include "assets/OptixMesh.h"
#include "assets/OptixInstancedModel.h"
#include "assets/OptixCubeMap.h"
#include "OptixUtil.h"

namespace Renderer::Optix
{
	static bool EnableRTX()
	{
		// rtx is on by default!
		int32 RTX = 1;

		return rtGlobalSetAttribute(RT_GLOBAL_ATTRIBUTE_ENABLE_RTX, sizeof(RTX), &RTX) == RT_SUCCESS;
	}

	OptixRendererBase::OptixRendererBase(System::Engine* context)
		: GLRendererBase(context), m_interopShaderProgram(0), m_interopGLTexture(0)
	{
		// alway try to enable rtx
		// TODO: remove this it is on by default, however check it
		static auto isRTXOn = EnableRTX();

		if (isRTXOn)
			RT_XENGINE_LOG_AT_LOWEST_LEVEL("Optix RTX execution mode is on!");
		else
			RT_XENGINE_LOG_FATAL("RTX mode is off!");


		m_optixContext = optix::Context::create();
		RT_XENGINE_ASSERT(m_optixContext, "optix context failed to init");


#ifdef DEBUG_OPTIX
		m_optixContext->setPrintEnabled(true);
		m_optixContext->setPrintBufferSize(1024);
#endif
	}

	OptixRendererBase::~OptixRendererBase()
	{
		m_optixContext->destroy();
	}

	bool OptixRendererBase::InitRendering(HWND assochWnd, HINSTANCE instance)
	{
		// create GL interop context
		GLRendererBase::InitRendering(assochWnd, instance);

		// OpenGL interop texture

		const char* vertexShader =
			R"(
		    #version 460 core
		    out vec2 UV;
		    void main()
		    {
		        float x = -1.0 + float((gl_VertexID & 1) << 2);
		        float y = -1.0 + float((gl_VertexID & 2) << 1);
		        UV.x = (x+1.0)*0.5;
		        UV.y = (y+1.0)*0.5;
		        gl_Position = vec4(x, y, 0, 1);
		    }
	    )";

		const char* fragmentShader =
			R"(
		    #version 460 core
		    in vec2 UV;
		    uniform sampler2D outTexture;
		    void main()
		    {
		        gl_FragColor = texture(outTexture, UV);
		    }
	    )";

		const GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &vertexShader, NULL);
		glCompileShader(vs);

		const GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, &fragmentShader, NULL);
		glCompileShader(fs);

		m_interopShaderProgram = glCreateProgram();
		glAttachShader(m_interopShaderProgram, vs);
		glAttachShader(m_interopShaderProgram, fs);

		glLinkProgram(m_interopShaderProgram);

		glDetachShader(m_interopShaderProgram, vs);
		glDetachShader(m_interopShaderProgram, fs);
		glDeleteShader(vs);
		glDeleteShader(fs);

		return true;
	}

	void OptixRendererBase::Render()
	{
		RTsize width, height;
		m_outBuffer->getSize(width, height);

		// dma transfer
		glBindTexture(GL_TEXTURE_2D, m_interopGLTexture);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_outBuffer->getGLBOId());
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE,
			nullptr);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		// write texture to screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, m_interopGLTexture);
		glUseProgram(m_interopShaderProgram);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glUseProgram(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		SwapBuffers();
	}

	void OptixRendererBase::WindowResize(int32 width, int32 height)
	{
		glBindTexture(GL_TEXTURE_2D, m_interopGLTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);

		glViewport(0, 0, width, height);
	}

	bool OptixRendererBase::InitScene(int32 width, int32 height)
	{
		glGenTextures(1, &m_interopGLTexture);
		glBindTexture(GL_TEXTURE_2D, m_interopGLTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);

		glViewport(0, 0, width, height);

		return true;
	}

	std::shared_ptr<OptixTexture> OptixRendererBase::RequestOptixTexture(Assets::Texture* texture)
	{
		return Assets::LoadAssetAtMultiKeyCache<OptixTexture>(m_optixTextures, this, texture);
	}

	std::shared_ptr<OptixCubeMap> OptixRendererBase::RequestOptixCubeMap(Assets::CubeMap* cubeMap)
	{
		return Assets::LoadAssetAtMultiKeyCache<OptixCubeMap>(m_optixCubeMaps, this, cubeMap);
	}

	std::shared_ptr<OptixModel> OptixRendererBase::RequestOptixModel(Assets::XModel* model, uint32 closestHitRayType,
	                                                                 Assets::StringFile* closestHitProgramSource, std::string closestHitProgramName, uint32 anyHitRayType,
	                                                                 Assets::StringFile* anyHitProgramSource, std::string anyHitProgramName)
	{
		return Assets::LoadAssetAtMultiKeyCache<OptixModel>(m_optixModels, this, model, closestHitRayType, closestHitProgramSource,
			closestHitProgramName, anyHitRayType, anyHitProgramSource, anyHitProgramName);
	}

	std::shared_ptr<OptixInstancedModel> OptixRendererBase::RequestOptixInstancedModel(uint32 closestHitRayType,
	    Assets::StringFile* closestHitProgramSource, std::string closestHitProgramName, uint32 anyHitRayType,
		Assets::StringFile* anyHitProgramSource, std::string anyHitProgramName, World::TriangleModelInstancedGeometryNode* nodeInstancer)
	{
		return Assets::LoadAssetAtMultiKeyCache<OptixInstancedModel>(m_optixInstancedModels, this, nodeInstancer->GetModel(), closestHitRayType, closestHitProgramSource,
			closestHitProgramName, anyHitRayType, anyHitProgramSource, anyHitProgramName, nodeInstancer);
	}

	std::shared_ptr<OptixMesh> OptixRendererBase::RequestOptixMesh(Assets::XMesh* mesh, uint32 closestHitRayType,
	                                                               Assets::StringFile* closestHitProgramSource, std::string closestHitProgramName, uint32 anyHitRayType,
	                                                               Assets::StringFile* anyHitProgramSource, std::string anyHitProgramName)
	{
		return Assets::LoadAssetAtMultiKeyCache<OptixMesh>(m_optixMeshes, this, mesh, closestHitRayType, closestHitProgramSource,
			closestHitProgramName, anyHitRayType, anyHitProgramSource, anyHitProgramName);
	}

	std::shared_ptr<OptixMaterial> OptixRendererBase::RequestOptixMaterial(Assets::XMaterial* material,
		uint32 closestHitRayType, Assets::StringFile* closestHitProgramSource, std::string closestHitProgramName,
		uint32 anyHitRayType, Assets::StringFile* anyHitProgramSource, std::string anyHitProgramName)
	{
		return Assets::LoadAssetAtMultiKeyCache<OptixMaterial>(m_optixMaterial, this, material, closestHitRayType, closestHitProgramSource,
			closestHitProgramName, anyHitRayType, anyHitProgramSource, anyHitProgramName);

	}

	std::shared_ptr<OptixProgram> OptixRendererBase::RequestOptixProgram(Assets::StringFile* file,
		const std::string& programName)
	{
		return Assets::LoadAssetAtMultiKeyCache<OptixProgram>(m_optixProgram, this, file, programName);
	}
}
