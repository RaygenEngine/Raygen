#pragma once

// types
#include "core/type/IntegerTypes.h"
#include "core/type/MathTypes.h"

// enums
#include "core/enum/InputEnums.h"
#include "core/enum/TargetEnums.h"
#include "core/enum/TextureEnums.h"
#include "core/enum/GeometryEnums.h"
#include "core/enum/BufferEnums.h"

// singletons
#include "core/timer/Timer.h"
#include "core/logger/Logger.h"
#include "core/uuid/UUIDGenerator.h"

// auxiliary
#include "core/auxiliary/GraphicsMathAux.h"
#include "core/auxiliary/MemoryAux.h"
#include "core/auxiliary/StringAux.h"

#include <bitset>
#include <fstream>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;
namespace ch = std::chrono;
