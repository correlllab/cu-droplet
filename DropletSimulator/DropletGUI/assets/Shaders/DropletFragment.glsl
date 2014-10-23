#version 330

//Fragment Shader for Droplet
//Applies per-vertex lighting to lit-up Droplet

out vec4 out_Color;

in vec4 aux_Color;
in vec3 transformedLightingDirection;
in vec3 normal;
in vec2 texCoords_0;
in vec3 directionalColor;
in vec3 ambientColor;


void main()
{
	vec3 model_Color = vec3(1.0);
	vec3 transformedNormal = normalize(normal);
	float directionalLightWeighting = max(dot(transformedNormal, normalize(transformedLightingDirection)), 0.0);
	vec3 vLightWeighting = ambientColor + directionalColor*directionalLightWeighting;
	vec3 light = 1 - (1 - vLightWeighting * (1-aux_Color.a)) * (1 - aux_Color.xyz * aux_Color.a);

	out_Color = vec4(model_Color * light,1.0);
}