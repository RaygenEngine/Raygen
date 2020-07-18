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


	// TODO: POSTPROC Probably grab from instance
	// const ReflClass* settingsClass;
	// void* settingsInstance;

	// Pt Entry sets (x | y | z)
	// Requires set
};
} // namespace vl
