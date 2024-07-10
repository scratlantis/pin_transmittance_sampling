#include "math.glsl"



float unitCubeExitDist(vec3 startPos, vec3 dir)
{
	assureNotZero(dir);
	vec3 invDir = 1.0 / dir;
	vec3 c1 = -startPos*invDir;
	vec3 c2 = c1 + invDir;
	return min(min(max(c1.x, c2.x), max(c1.y, c2.y)), max(c1.z, c2.z));
}

void unitCubeIntersection(vec3 origin, vec3 direction,inout vec3 entry, inout vec3 exit)
{
	vec3 dir = direction;
	assureNotZero(dir);
	vec3 invDir = 1.0 / dir;
	vec3 c1 = -origin*invDir;
	vec3 c2 = c1 + invDir;
	vec3 cMin = min(c1,c2);
	vec3 cMax = max(c1,c2);
	float tMin = max(max(cMin.x,cMin.y), cMin.z);
	float tMax = min(min(cMax.x, cMax.y), cMax.z); 
	entry = origin + tMin * dir;
	exit = origin + tMax * dir;
}