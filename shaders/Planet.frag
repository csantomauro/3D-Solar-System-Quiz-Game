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

// layout(binding = 3) uniform sampler2D texEmit;

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

vec3 BRDF(vec3 V, vec3 N, vec3 L, vec3 Md, float F0, float metallic, float roughness) {
	//vec3 V  - direction of the viewer
	//vec3 N  - normal vector to the surface
	//vec3 L  - light vector (from the light model)
	//vec3 Md - main color of the surface
	//float F0 - Base color for the Fresnel term
	//float metallic - parameter that mixes the diffuse with the specular term.
	//                 in particular, parmeter K seen in the slides is: float K = 1.0f - metallic;
	//float roughness - Material roughness (parmeter rho in the slides).
	//specular color Ms is not passed, and implicitely considered white: vec3 Ms = vec3(1.0f);

	float Pi = 3.1415926;
	vec3 Ms = vec3(1.0f);
	float K = 1.0f - metallic;

	vec3 Hlx = normalize(L + V);
	float term1 = pow((clamp(dot(Hlx,N), 0.01, 1)), 2);
	float term2 = roughness * roughness -1;
	float D = (roughness * roughness) / (Pi * pow((term1 * term2 + 1), 2));
	float F = F0 + (1 - F0) * pow((1 - clamp(dot(Hlx, V), 0.01, 1)), 5);
	float G1 = 2/(1 + sqrt(1 + roughness * roughness * (1 - pow(dot(V,N), 2))/(pow(dot(V,N), 2))));
	float G2 = 2/(1 + sqrt(1 + roughness * roughness * (1 - pow(dot(L,N), 2))/(pow(dot(L,N), 2))));
	float G = G1 * G2;

	vec3 specular = Ms * (D*F*G)/(4 * clamp(dot(V,N), 0.01, 1)); 
	vec3 diffuse = Md * clamp(dot(L,N), 0.01, 1);

	return K * diffuse + (1.0f - K)*specular;
}

void main() {
	vec3 N = normalize(fragNorm);				// surface normal
	vec3 V = normalize(gubo.eyePos - fragPos);	// viewer direction
	vec3 MD = texture(tex, fragUV).rgb;			// diffuse color
	vec3 MA = MD;								// ambient color
	vec3 MS = vec3(1);							// specular color
	vec3 ME = vec3(0); // texture(texEmit, fragUV).rgb;		// emission color

    // pointlight
	vec3 lightColor = lightModelColor();
	vec3 L = lightModelDirection();

	vec3 albedo = texture(tex, fragUV).rgb;

	float roughness = 0.2f;
	float ao = 0.2f;
	float metallic = 0.2f;

	vec3 DiffSpec = BRDF(V, N, L, albedo, 0.3f, metallic, roughness);
	vec3 Ambient = albedo * 0.05f * ao;
	
	outColor = vec4(clamp(0.95 * DiffSpec * lightColor.rgb + Ambient,0.0,1.0), 1.0f);
}