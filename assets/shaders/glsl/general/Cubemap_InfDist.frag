#version 460 core

out vec4 out_color;

in vec3 tex_coords;

layout(binding=0) uniform samplerCube skybox;

void main()
{    
    out_color = texture(skybox, tex_coords);
}