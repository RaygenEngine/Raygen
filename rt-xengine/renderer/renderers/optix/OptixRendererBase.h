#ifndef OPTIXRENDERERBASE_H
#define OPTIXRENDERERBASE_H

#include <Optix/optixu/optixpp_namespace.h>

#include "renderer/Renderer.h"
#include "assets/xasset/XModel.h"
#include "assets/other/utf8/StringFile.h"

#include "assets/MultiKeyAssetCacheHashing.h"
#include "renderer/renderers/opengl/GLRendererBase.h"

//#define DEBUG_OPTIX
#define SCENE_EPSILON 1.e-3f

namespace Renderer::Optix
{
	class OptixTexture;
	class OptixModel;
	class OptixInstancedModel;
	class OptixMesh;
	class OptixMaterial;
	class OptixProgram;
	class OptixCubeMap;

							  // ATTENTION: this is a GL renderer base sub-base because:
							  // 1) we need opengl rendering for context interop
	                          // 2) we need gl assets etc
	class OptixRendererBase : public OpenGL::GLRendererBase
	{
	protected:

		// Optix stuff 
		optix::Context m_optixContext;
		optix::Buffer m_outBuffer;

		// GL stuff
		// shader program for texure to window rendering
		GLuint m_interopShaderProgram;
		// interop texture
		GLuint m_interopGLTexture;

		Assets::MultiKeyAssetCache<OptixTexture, Assets::Texture*> m_optixTextures{};
		Assets::MultiKeyAssetCache<OptixCubeMap, Assets::CubeMap*> m_optixCubeMaps{};
		Assets::MultiKeyAssetCache<OptixModel, Assets::XModel*, uint32, Assets::StringFile*, std::string, uint32, Assets::StringFile*, std::string> m_optixModels{};
		Assets::MultiKeyAssetCache<OptixMesh, Assets::XMesh*, uint32, Assets::StringFile*, std::string, uint32, Assets::StringFile*, std::string> m_optixMeshes{};
		Assets::MultiKeyAssetCache<OptixMaterial, Assets::XMaterial*, uint32, Assets::StringFile*, std::string, uint32, Assets::StringFile*, std::string> m_optixMaterial{};
		Assets::MultiKeyAssetCache<OptixProgram, Assets::StringFile*, std::string> m_optixProgram{};
		Assets::MultiKeyAssetCache<OptixInstancedModel, Assets::XModel*, uint32, Assets::StringFile*, std::string, uint32, Assets::StringFile*, std::string, World::TriangleModelInstancedGeometryNode*> m_optixInstancedModels{};

		OptixRendererBase(System::Engine* context);
		virtual ~OptixRendererBase();

	public:
		bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
		// Init OpenGL interop
		bool InitScene(int32 width, int32 height) override;
		// Render with OpenGL interop
		void Render() override;
		// Default resize behaviour resizes the output buffer and the interop texture
		void WindowResize(int32 width, int32 height) override;

		std::shared_ptr<OptixTexture> RequestOptixTexture(Assets::Texture* texture);

		std::shared_ptr<OptixCubeMap> RequestOptixCubeMap(Assets::CubeMap* texture);

		std::shared_ptr<OptixModel> RequestOptixModel(Assets::XModel* model, uint32 closestHitRayType,
			Assets::StringFile* closestHitProgramSource,
			std::string closestHitProgramName, uint32 anyHitRayType,
			Assets::StringFile* anyHitProgramSource, std::string anyHitProgramName);

		std::shared_ptr<OptixInstancedModel> RequestOptixInstancedModel(uint32 closestHitRayType,
		                                                                Assets::StringFile* closestHitProgramSource, std::string closestHitProgramName, uint32 anyHitRayType,
		                                                                Assets::StringFile* anyHitProgramSource, std::string anyHitProgramName, World::TriangleModelInstancedGeometryNode* nodeInstancer);

		std::shared_ptr<OptixMesh> RequestOptixMesh(Assets::XMesh* mesh, uint32 closestHitRayType,
		                                            Assets::StringFile* closestHitProgramSource, std::string closestHitProgramName, uint32 anyHitRayType,
		                                            Assets::StringFile* anyHitProgramSource, std::string anyHitProgramName);

		std::shared_ptr<OptixMaterial> RequestOptixMaterial(Assets::XMaterial* material, uint32 closestHitRayType,
		                                                    Assets::StringFile* closestHitProgramSource, std::string closestHitProgramName, uint32 anyHitRayType,
		                                                    Assets::StringFile* anyHitProgramSource, std::string anyHitProgramName);

		std::shared_ptr<OptixProgram> RequestOptixProgram(Assets::StringFile* sourceFile, const std::string& programName);

		optix::Context GetOptixContext() const { return m_optixContext; }
	};
}

#endif // OPTIXRENDERERBASE_H
