#ifndef GLMATERIAL_H
#define GLMATERIAL_H

#include "renderer/renderers/opengl/GLAsset.h"
#include "GLTexture.h"


namespace Renderer::OpenGL
{
	class GLMaterial : public GLAsset 
	{
		// RGB: Albedo A: Opacity
		std::shared_ptr<GLTexture> m_textSurfaceAlbedo;
		// RGB: Emission A: Ambient Occlusion
		std::shared_ptr<GLTexture> m_textSurfaceEmission;
		// R: Reflectivity G: Roughness B: Metallic A: Translucency
		std::shared_ptr<GLTexture> m_textSurfaceSpecularParameters;
		// RGB: Normal A: Height
		std::shared_ptr<GLTexture> m_textSurfaceBump;

	public:

		GLMaterial(GLRendererBase* renderer);
		~GLMaterial() = default;

		bool Load(Assets::XMaterial* data);

		GLTexture* GetTextureSurfaceAlbedo() const { return m_textSurfaceAlbedo.get(); }
		GLTexture* GetTextureSurfaceEmission() const { return m_textSurfaceEmission.get(); }
		GLTexture* GetTextureSurfaceSpecularParameters() const { return m_textSurfaceSpecularParameters.get(); }
		GLTexture* GetTextureSurfaceBump() const { return m_textSurfaceBump.get(); }

		void ToString(std::ostream& os) const override { os << "asset-type: GLMaterial, name: " << m_associatedDescription; }
	};

}

#endif // GLMATERIAL_H
