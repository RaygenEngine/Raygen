#pragma once

#include "renderer/GenericGpuAssetManager.h"
#include "renderer/renderers/opengl/GLAsset.h"

namespace ogl {
class GLAssetManager : public GenericGpuAssetManager<GLAssetBase> {
};
} // namespace ogl
