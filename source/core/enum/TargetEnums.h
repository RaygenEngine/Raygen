#pragma once

enum class LogLevelTarget { TRACE = 0, DEBUG, INFO, WARN, ERR, CRITICAL, OFF };

enum PreviewTarget : int32 {
	// base color
	PT_OUT,
	PT_BASE_COLOR_MAP,
	PT_BASE_COLOR_FACTOR,
	PT_BASE_COLOR_FINAL,
	// metallic
	PT_METALLIC_MAP,
	PT_METALLIC_FACTOR,
	PT_METALLIC_FINAL,
	// roughness
	PT_ROUGHNESS_MAP,
	PT_ROUGHNESS_FACTOR,
	PT_ROUGHNESS_FINAL,
	// normal
	PT_NORMAL,
	PT_NORMAL_SCALE,
	PT_NORMAL_MAP,
	PT_NORMAL_FINAL,
	// tangent
	PT_TANGENT,
	PT_TANGENT_HANDEDNESS,
	// bitangent
	PT_BITANGENT,
	// occlusion
	PT_OCCLUSION_MAP,
	PT_OCCLUSION_STRENGTH,
	PT_OCCLUSION_FINAL,
	// emissive
	PT_EMISSIVE_MAP,
	PT_EMISSIVE_FACTOR,
	PT_EMISSIVE_FINAL,
	// opacity
	PT_OPACITY_MAP,
	PT_OPACITY_FACTOR,
	PT_OPACITY_FINAL,
	// texCoords
	PT_TEXTURE_COORDINATE0,
	PT_TEXTURE_COORDINATE1,
	// alpha
	PT_ALPHA_MODE,
	PT_ALPHA_CUTOFF,
	PT_ALPHA_MASK,
	// double sidedness
	PT_DOUBLE_SIDEDNESS,
	PT_COUNT
};

namespace utl {
inline const char* SurfacePreviewTargetModeString(int32 pt)
{
	switch (pt) {
		case PT_OUT: return "out";
		case PT_BASE_COLOR_MAP: return "base color map";
		case PT_BASE_COLOR_FACTOR: return "base color factor";
		case PT_BASE_COLOR_FINAL: return "base color final";
		case PT_METALLIC_MAP: return "metallic map";
		case PT_METALLIC_FACTOR: return "metallic factor";
		case PT_METALLIC_FINAL: return "metallic final";
		case PT_ROUGHNESS_MAP: return "roughness map";
		case PT_ROUGHNESS_FACTOR: return "roughness factor";
		case PT_ROUGHNESS_FINAL: return "roughness final";
		case PT_NORMAL: return "normal";
		case PT_NORMAL_SCALE: return "normal scale";
		case PT_NORMAL_MAP: return "normal map";
		case PT_NORMAL_FINAL: return "normal final";
		case PT_TANGENT: return "tangent";
		case PT_TANGENT_HANDEDNESS: return "tangent handedness";
		case PT_BITANGENT: return "bitangent";
		case PT_OCCLUSION_MAP: return "occlusion map";
		case PT_OCCLUSION_STRENGTH: return "occlusion strength";
		case PT_OCCLUSION_FINAL: return "occlusion final";
		case PT_EMISSIVE_MAP: return "emissive map";
		case PT_EMISSIVE_FACTOR: return "emissive factor";
		case PT_EMISSIVE_FINAL: return "emissive final";
		case PT_OPACITY_MAP: return "opacity map";
		case PT_OPACITY_FACTOR: return "opacity factor";
		case PT_OPACITY_FINAL: return "opacity final";
		case PT_TEXTURE_COORDINATE0: return "texture coordinate 0";
		case PT_TEXTURE_COORDINATE1: return "texture coordinate 1";
		case PT_ALPHA_MODE: return "alpha mode (red: opaque, green: mask, blue: blend)";
		case PT_ALPHA_CUTOFF: return "alpha cutoff";
		case PT_ALPHA_MASK: return "alpha mask (if mask enabled> black: opaque, white: transparent)";
		case PT_DOUBLE_SIDEDNESS: return "double sidedness (black: false, white: true)";
		default: return "invalid";
	}
}
} // namespace utl