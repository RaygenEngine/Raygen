#version 460 core

out vec4 out_color;

in vec3 textCoord;

layout(binding=0) uniform samplerCube cubemap;

void main()
{    
    out_color = texture(cubemap, textCoord);
}