#include "test.h"
#include "engine/Logger.h"
#include "reflection/PodReflection.h"


#include "assets/AssetPod.h"


TEST("Pod Reflection")
{
	AssetPod* pod{ nullptr };
	if (0) { // We just need to check if this compiles
			 // TEST: Sgould compile while include including only the specific pod.
			 // auto o = PodCast<StringPod>(pod);
	}
}
