#pragma once

#include "rendering/ppt/PtBase.h"
#include "reflection/TypeId.h"

namespace vl {

struct PtEntry {
	UniquePtr<PtBase> instance;
	TypeId type;

	bool isEnabled{ true };
	size_t entryIndex;

	std::function<PtBase*()> constructor;

	// const ReflClass* settingsClass;
	// void* settingsInstance; // WIP: Probably grab from instance

	// TODO: Pt Entry sets (x | y | z)
	// TODO: Requires set
};
} // namespace vl