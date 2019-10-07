#version 460 core

layout (location = 0) in vec3 pos;

out vec3 tex_coords;

// view projection, transformation is omitted from view
uniform mat4 vp;

void main()
{
    tex_coords = pos;
    vec4 fpos = vp * vec4(pos, 1.0);
	// inf distance
    gl_Position = fpos.xyww;
}  