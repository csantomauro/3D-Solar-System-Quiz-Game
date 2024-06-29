#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
	float x;
} ubo;

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec2 outPosition;

void main() {
	gl_Position = vec4(inPosition[0]+ubo.x, inPosition[1], 0.5f, 1.0f);
	outPosition = inPosition;
}