struct GLSLMediumInstance
{
	mat4 mat;
	mat4 invMat;
	vec3 albedo;
	uint id;

	uint cullMask;
	uint padding[3];
};