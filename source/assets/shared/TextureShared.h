#pragma once

enum class TextureFiltering
{
	Nearest,
	Linear,
};

enum class MipmapFiltering
{
	Nearest,
	Linear,
	NoMipmap
};

// CHECK: (mirrored clamping etc)
enum class TextureWrapping
{
	ClampToEdge,
	MirroredRepeat,
	Repeat
};
