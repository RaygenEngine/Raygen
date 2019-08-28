#pragma once

typedef enum
{
	GM_POINTS,
	GM_LINE,
	GM_LINE_LOOP,
	GM_LINE_STRIP,
	GM_TRIANGLES,
	GM_TRIANGLE_STRIP,
	GM_TRIANGLE_FAN,
	GM_INVALID
} GeometryMode;

typedef enum
{
	GU_DYNAMIC,
	GU_STATIC,
	GU_INVALID
} GeometryUsage;

typedef enum
{
	// The rendered output is fully opaque and any alpha value is ignored.
	AM_OPAQUE,
	// The rendered output is either fully opaque or fully transparent depending on the alpha value and the specified alpha cutoff value.
	// This mode is used to simulate geometry such as tree leaves or wire fences.
	AM_MASK,
	// The rendered output is combined with the background using the normal painting operation (i.e. the Porter and Duff over operator).
	// This mode is used to simulate geometry such as guaze cloth or animal fur.
	AM_BLEND,
	AM_INVALID
} AlphaMode;
