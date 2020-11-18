
#ifndef mainpass_inputs_glsl
#define mainpass_inputs_glsl

// GBuffer
layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput g_DepthInput;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput g_NormalInput;
layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput g_AlbedoInput;
layout (input_attachment_index = 3, set = 0, binding = 3) uniform subpassInput g_SpecularInput;
layout (input_attachment_index = 4, set = 0, binding = 4) uniform subpassInput g_EmissiveInput;
layout (input_attachment_index = 5, set = 0, binding = 5) uniform subpassInput g_VelocityInput;
layout (input_attachment_index = 6, set = 0, binding = 6) uniform subpassInput g_UVDrawIndexInput;


#endif
