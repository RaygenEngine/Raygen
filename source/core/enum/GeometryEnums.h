#pragma once

enum class GeometryMode { POINTS, LINE, LINE_LOOP, LINE_STRIP, TRIANGLES, TRIANGLE_STRIP, TRIANGLE_FAN, INVALID };

enum class GeometryUsage { DYNAMIC, STATIC, INVALID };

enum AlphaMode : int32 {
	// The rendered output is fully opaque and any alpha value is ignored.
	AM_OPAQUE,
	// The rendered output is either fully opaque or fully transparent depending on the alpha value and the specified
	// alpha cutoff value. This mode is used to simulate geometry such as tree leaves or wire fences.
	AM_MASK,
	// The rendered output is combined with the background using the normal painting operation (i.e. the Porter and Duff
	// over operator). This mode is used to simulate geometry such as guaze cloth or animal fur.
	AM_BLEND,
	AM_INVALID
};
