#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "Types.h"

struct XFov
{
	// between viewing vector and top edge of fov
	float topTan; 
	// between viewing vector and bottom edge of fov
	float bottomTan; 
	// between viewing vector and right edge of fov
	float rightTan;
	// between viewing vector and left edge of fov
	float leftTan; 
};

typedef enum
{
	ET_LEFT = 0,
	ET_RIGHT = 1,
	ET_COUNT = 2
} EyeTarget;

struct Viewport
{
	int32 x;
	int32 y;
	int32 width;
	int32 height;
};

// single texture shared in half (width) by each eye
// holds max of y/x fovs and eyes have symmetrical and equal fovs
struct GLOculusSingleTextureSymmetricalFov
{
	// eyes texture width
	uint32 width;
	// eyes texture height
	uint32 height;
	float aspectRatio;

	float _near;
	float _far;

	float yfovHalfTan;
	float xfovHalfTan;

	uint32 glTexture;
	uint32 glDepth;
};

struct WindowTarget
{
	uint32 width;
	uint32 height;
	float aspectRatio;
};

struct RenderTarget
{
	uint32 type;

	WindowTarget window;

	union
	{
		uint32 glTexture;
		void *mapMem;
		// single texture for both eyes
		GLOculusSingleTextureSymmetricalFov glOculusSingleTextureSymmetricalFov;
	};
};

#endif // RENDERTARGET_H
