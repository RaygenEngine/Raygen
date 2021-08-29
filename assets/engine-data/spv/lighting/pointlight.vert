layout(location = 0) noperspective out vec2 uv;

layout(location = 0) in vec3 position;

layout(push_constant) uniform PC {
	mat4 lightVolMatVP;
} push;


void main() 
{
    gl_Position = push.lightVolMatVP * vec4(position, 1.0);

    uv = gl_Position.xy / gl_Position.w;
	uv = uv.xy * 0.5 + 0.5;
}
