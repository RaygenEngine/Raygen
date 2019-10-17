#ifndef random_h
#define random_h

float random(vec4 seed4)
{
	float dot = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot) * 43758.5453);
}

#endif