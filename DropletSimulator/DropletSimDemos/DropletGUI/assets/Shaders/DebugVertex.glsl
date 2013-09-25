#version 330

//Debug mode vertex shader
//Light calculations to apply per-vertex lighting

// Input vertex data, different for all executions of this shader.
layout(location=0) in vec4 in_vertexPos;
layout(location=1) in vec3 in_Normals; //Lighting normals on objs
layout(location=2) in vec2 in_Texture; 

//Uniforms for constructing model-view matrix
uniform mat4 in_Projection;
uniform mat4 in_View;
uniform mat4 in_Model;	
uniform vec4 in_Color;
uniform vec3 in_lightDir;
uniform vec2 in_ProjOffsets;

//Info to fragment shader
out vec4 ex_Color;
out vec3 vLightWeighting; //Lighting

//Lighting
	vec3 ambientColor = vec3(0.4);
	vec3 directionalColor = vec3(0.4);

void main(){
	mat4 modelView = in_View * in_Model;

	//Lighting to be passed into fragment shader.
	vec3 transformedNormal = normalize((modelView * vec4(in_Normals, 0.0)).xyz);
	vec3 transformedLightingDirection = normalize(vec3(in_View * vec4(in_lightDir,0.0)));
	float directionalLightWeighting = max(dot(transformedNormal, transformedLightingDirection), 0.0);
	vLightWeighting = ambientColor + directionalColor*directionalLightWeighting;
	gl_Position = in_Projection * (modelView * in_vertexPos);

	ex_Color = in_Color;
	
}