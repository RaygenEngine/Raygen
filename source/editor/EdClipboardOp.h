#pragma once

#include "ecs_universe/Entity.h"
#include "reflection/GetClass.h"

namespace ed {

// Implementations actually use OS clipboard buffer (for now?) to allow external text editing. (Can be usefull
// sometimes). Implementations are relatively slow as we encode everything to json (custom format later? or binary?)
struct ClipboardOp {
	static void StoreEntity(Entity ent);

	// Returns null entity if paste was invalid
	static Entity LoadEntity(entt::registry& reg);


	// CHECK: Const without const cast here, fix the const-correctness in void* reflection functions
	template<typename T>
	static void StoreStructure(T& data)
	{
		StoreStructure(&data, refl::GetClass(&data));
	}


	// Returns true if successful. CHECK: Can return ReflCopyOperation
	template<typename T>
	static void LoadStructure(T& data)
	{
		LoadStructure(&data, refl::GetClass(&data));
	}


	static void StoreStructure(void* data, const ReflClass& cl);
	static void LoadStructure(void* data, const ReflClass& cl);
};

} // namespace ed
