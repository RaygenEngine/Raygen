#version 460 core

out vec4 out_color;

in vec2 uv;

uniform float near;
uniform float far;

layout(binding=0) uniform sampler2D outTexture;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{             
    float depth = LinearizeDepth(texture(outTexture, uv).r) / far; // divide by far for demonstration
    out_color = vec4(vec3(depth), 1.0);
}