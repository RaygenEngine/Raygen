#version 460

// out

layout(location = 0) noperspective out vec2 uv;

// in

layout(location = 0) in vec3 position;

// uniforms

layout(push_constant) uniform PC {
	mat4 sphereVolMatVp;
};

void main() 
{
    gl_Position = sphereVolMatVp * vec4(position, 1.0);

    uv = gl_Position.xy / gl_Position.w;
	uv = uv.xy * 0.5 + 0.5;
}
