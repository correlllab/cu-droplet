#version 330
//Texture Fragment shader
//Applies per-vertex lighting to textured objects.

uniform sampler2D objectTexture;

//To fragment shader
out vec4 out_Color;

in vec4 model_Color;
in vec3 transformedLightingDirection;
in vec3 normal;
in vec2 texCoords_0;
in vec3 directionalColor;
in vec3 ambientColor;


void main()
{
	vec4 tex_Color = texture2D(objectTexture,texCoords_0);

	vec3 transformedNormal = normalize(normal);
	float directionalLightWeighting = max(dot(transformedNormal, normalize(transformedLightingDirection)), 0.0);
	vec3 vLightWeighting = ambientColor + directionalColor*directionalLightWeighting;

	out_Color = vec4(tex_Color.xyz * vLightWeighting, 1.0); //Applies per-vertex lighting to objects

}

