#include "math.glsl"



float unitCubeExitDist(vec3 startPos, vec3 dir)
{
	assureNotZero(dir);
	vec3 invDir = 1.0 / dir;
	vec3 c1 = -startPos*invDir;
	vec3 c2 = c1 + invDir;
	return min(min(max(c1.x, c2.x), max(c1.y, c2.y)), max(c1.z, c2.z));
}
