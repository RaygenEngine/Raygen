#version 460

// out

layout (location = 0) out vec2 uv;

// in

layout(location = 0) in vec3 position;

// uniforms

layout(push_constant) uniform PC {
    mat4 vp;
    vec4 centerPos;
    vec4 cameraRight;
    vec4 cameraUp;
    vec2 uvstart;
    vec2 uvend;
};


void main() 
{
	float scale = 0.2; // TODO: Calculate from camera distance
    vec3 pos = centerPos.xyz
    + cameraRight.xyz * position.x * scale
    + cameraUp.xyz * position.y * scale;

    gl_Position = vp * vec4(pos, 1.0);


    float u = sign(position.x) < 0 ? uvstart.x : uvend.x;
    float v = sign(position.y) > 0 ? uvstart.y : uvend.y;
    uv = vec2(u, v);
}                


