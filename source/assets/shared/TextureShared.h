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

enum class TextureWrapping
{
	ClampToEdge,
	MirroredRepeat,
	Repeat
};
