#pragma once

enum class MaterialAlphaMode : int32
{
	Opaque, // Opaque surface
	Mask,   // Opaque surface with mask
	Blend   // transparent surface
};
