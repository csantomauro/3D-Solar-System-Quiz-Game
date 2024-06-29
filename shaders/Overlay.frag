#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D tex;

void main() {
	//outColor = vec4(texture(tex, fragUV).rgb, 1.0f);	// output color

	vec4 texColor = texture(tex, fragUV);
    outColor = texColor; // Start with the color from the texture

    // Use the alpha channel from the texture
    outColor.a = texColor.a;

    // To discard fully transparent fragments:
    if (outColor.a < 0.01) discard;
}