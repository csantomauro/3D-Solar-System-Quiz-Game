#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D tex;

layout(set = 0, binding = 2) uniform GlobalUniformBufferObject {
	vec3 lightPos;	// position of the point light
	vec4 lightColor;// color of the point light
	vec3 AmbLightColor;	// ambient light
	vec3 eyePos;	// position of the viewer
} gubo;

layout(set = 0, binding = 3) uniform sampler2D texEmit;

const float beta = 2.0f;	// decay exponent of the point light
const float g = 20;		// target distance of the point light

vec3 lightModelColor() {
    // point light
    float tmp = g/length(gubo.lightPos - fragPos);
	return vec3(gubo.lightColor)*pow(tmp, beta);
}

vec3 lightModelDirection() {
    return normalize(gubo.lightPos - fragPos);
}

vec3 LambertDiffuse(vec3 L, vec3 N, vec3 Md) {
	vec3 f_diffuse = Md*max(dot(L,N),0);
	return f_diffuse;
}

vec3 AmbientLight(vec3 La, vec3 Ma, vec3 Me) {
	return Ma * La + Me;
}

void main() {
	vec3 N = normalize(fragNorm);				// surface normal
	vec3 V = normalize(gubo.eyePos - fragPos);	// viewer direction
	vec3 MD = texture(tex, fragUV).rgb;			// diffuse color
	vec3 MA = MD;								// ambient color
	vec3 MS = vec3(1);							// specular color
	vec3 ME = texture(texEmit, fragUV).rgb;		// emission color

    // pointlight
	vec3 lightColor = lightModelColor();
	vec3 L = lightModelDirection();

    // lambert
	vec3 DiffSpec = LambertDiffuse(L, N, MD);
	vec3 Ambient = AmbientLight(gubo.AmbLightColor, MA, ME);	

	outColor = vec4(clamp(0.95 * DiffSpec * lightColor.rgb + Ambient,0.0,1.0), 1.0f);
}