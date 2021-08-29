layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 uv;

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;

void main( ) {
	vec3 color = subpassLoad(inputColor).rgb;
	outColor = vec4(vec3(1.0)-color, 1.0);
	outColor = subpassLoad(inputColor);
}                                                                                                                          

