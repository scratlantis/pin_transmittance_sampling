vec2 getEnvMapTexCoord(vec3 dir)
{
	vec3 viewDir = dir;
	viewDir = vec3(viewDir.y,-viewDir.x,viewDir.z);
	viewDir = vec3(viewDir.z, viewDir.y, -viewDir.x);
	float theta = atan(viewDir.y/viewDir.x);
	float phi = atan(length(viewDir.xy), viewDir.z);
	vec2 texCoords;
	texCoords.x = theta/(PI);
	texCoords.y = phi/PI;
	return texCoords;
}