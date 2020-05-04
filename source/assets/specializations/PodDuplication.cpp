#include "pch.h"
#include "PodDuplication.h"

#include "reflection/ReflectionTools.h"
#include "reflection/PodTools.h"

namespace podspec {
void Duplicate(AssetPod* src, AssetPod* dst)
{
	auto result = refltools::CopyClassTo(src, dst);
	CLOG_ERROR(!result.IsExactlyCorrect(), "Duplicate pod had errors! Are you sure the pods are the same type?");

	podtools::VisitPod(src, [dst]<typename T>(T* src_cast) { //
		DuplicateData<T>(src_cast, static_cast<T*>(dst));
	});
}
} // namespace podspec
