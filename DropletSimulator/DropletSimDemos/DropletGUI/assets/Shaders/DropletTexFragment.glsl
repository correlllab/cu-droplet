#version 330
//Texture Fragment Shader for Droplet (no projector)
//Applies per-vertex lighting to textured Droplet

uniform sampler2D objectTexture;

//To fragment shader
out vec4 out_Color;

in vec4 aux_Color;
in vec3 transformedLightingDirection;
in vec3 normal;
in vec2 texCoords_0;
in vec3 directionalColor;
in vec3 ambientColor;

//Lighting


void main()
{
	vec4 tex_Color = texture2D(objectTexture,texCoords_0);

	vec3 transformedNormal = normalize(normal);
	float directionalLightWeighting = max(dot(transformedNormal, normalize(transformedLightingDirection)), 0.0);
	vec3 vLightWeighting = ambientColor + directionalColor*directionalLightWeighting;
	vec3 light = 1 - (1 - vLightWeighting * (1-aux_Color.a)) * (1 - aux_Color.xyz*tex_Color.a);

	out_Color = vec4(tex_Color.xyz * light,1.0);
}