#pragma once

// types
#include "core/type/IntegerTypes.h"
#include "core/type/MathTypes.h"

// enums
#include "core/enum/InputEnums.h"
#include "core/enum/TargetEnums.h"
#include "core/enum/TextureEnums.h"
#include "core/enum/GeometryEnums.h"

// c friendly data structs
#include "core/data/BasicLight.h"
#include "core/data/Vertex.h"

// singletons
#include "core/timer/Timer.h"
#include "core/logger/Logger.h"
#include "core/uuid/UUIDGenerator.h"

// auxiliary
#include "core/auxiliary/FileAux.h"
#include "core/auxiliary/GraphicsMathAux.h"
#include "core/auxiliary/SmartPtrAux.h"
#include "core/auxiliary/StringAux.h"
#include "core/auxiliary/CachingAux.h"

// contexts
#include "input/Input.h"
#include "assets/DiskAssetManager.h"
#include "world/World.h"
#include "renderer/Renderer.h"
#include "system/Engine.h"
