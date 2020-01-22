#pragma once
// Utility header that includes all pods.
// All pod headers are required in a decent amount of files (especially reflection) therefore a central
// location to include them is prefered over changing multiple cpp files when a new pod is added.

#include "asset/pods/GltfFilePod.h"
#include "asset/pods/ImagePod.h"
#include "asset/pods/MaterialPod.h"
#include "asset/pods/ModelPod.h"
#include "asset/pods/ShaderPod.h"
#include "asset/pods/StringPod.h"
#include "asset/pods/TexturePod.h"
#include "asset/pods/JsonDocPod.h"
#include "asset/pods/BinaryPod.h"
