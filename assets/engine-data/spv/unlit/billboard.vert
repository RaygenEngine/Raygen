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
    float scale;
};


void main() 
{
    vec3 pos = centerPos.xyz
    + cameraRight.xyz * position.x * scale
    + cameraUp.xyz * position.y * scale;

    gl_Position = vp * vec4(pos, 1.0);
    
    uv = vec2(((gl_VertexIndex) << 1) & 2, (gl_VertexIndex) & 2);
}                
                

