#version 330

//Debug mode fragment shader
//Applies simple per-vertex lighting to colored object

in vec4 ex_Color;
out vec4 out_Color;
in vec3 vLightWeighting; //Lighting

void main()
{ 
	out_Color = vec4(ex_Color.xyz * vLightWeighting, 1.0); //Applies per-vertex lighting to objects
}