#ifndef random_h
#define random_h

float random(vec4 seed4)
{
	float dot = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot) * 43758.5453);
}

// Based omn http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
float random(vec2 co)
{
	float a = 12.9898;
	float b = 78.233;
	float c = 43758.5453;
	float dt= dot(co.xy ,vec2(a,b));
	float sn= mod(dt,3.14);
	return fract(sin(sn) * c);
}

#endif