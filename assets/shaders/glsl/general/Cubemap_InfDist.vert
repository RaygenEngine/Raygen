#version 460 core

layout (location = 0) in vec3 pos;

out vec3 textCoord;

// view projection, transformation is omitted from view
uniform mat4 vp;

void main()
{
    textCoord = pos;
    vec4 fpos = vp * vec4(pos, 1.0);
	// inf distance
    gl_Position = fpos.xyww;
}  