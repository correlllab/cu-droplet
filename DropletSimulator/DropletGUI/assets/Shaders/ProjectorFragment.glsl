#version 330
//Projector Fragment Shader
//Applies per-vertex lighting to textured objects, and applies projection down onto the object (if below projected image).

uniform sampler2D objectTexture; //The texture of the object
uniform sampler2D projectionTexture; //The texture holding the projected image

//To fragment shader
out vec4 out_Color;

in vec4 model_Color;
in vec3 transformedLightingDirection;
in vec3 transformedProjDirection;
in vec3 normal;
in vec2 texCoords_0;
in vec2 texCoords_1;
in vec3 directionalColor;
in vec3 ambientColor;


void main()
{

	vec4 tex_Color = texture2D(objectTexture,texCoords_0);
	vec4 proj_Color = vec4(0.0);
	if (all(greaterThanEqual(texCoords_1,vec2(0.0)))&&all(lessThanEqual(texCoords_1,vec2(1.0))))
		proj_Color = texture2D(projectionTexture,texCoords_1);

	vec3 transformedNormal = normalize(normal);
	float directionalLightWeighting = max(dot(transformedNormal, normalize(transformedLightingDirection)), 0.0);
	float projLightWeighting = max(dot(transformedNormal,normalize(transformedProjDirection)),0.0);
	vec3 vLightWeighting = ambientColor + directionalColor*directionalLightWeighting + proj_Color.xyz*projLightWeighting;

	out_Color = vec4( tex_Color.xyz * vLightWeighting, 1.0); //Applies per-vertex lighting to objects

}