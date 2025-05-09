
#ifndef MATH_H
#define MATH_H

#define PI		3.141592
#define INV_PI  0.318310
#define INV_4PI  0.079577

#define EPSILON	0.0001

#define MAX_FLOAT intBitsToFloat(2139095039)

#define SQRT3 1.73205080757

mat4x3 tangentFrameFromPosNormal(vec3 pos, vec3 normal)
{
	vec3 tangent = (cross(normal, vec3(1.0, 0.0, 0.0)));
	if (length(tangent) < 0.1)
	{
		tangent = (cross(normal, vec3(0.0, 1.0, 0.0)));
	}
	tangent = normalize(tangent);
	vec3 bitangent = normalize(cross(normal, tangent));
	return mat4x3(tangent, bitangent, normal, pos);
}

mat3x3 tangentFrameFromNormal(vec3 normal)
{
	vec3 tangent = (cross(normal, vec3(1.0, 0.0, 0.0)));
	if (length(tangent) < 0.1)
	{
		tangent = (cross(normal, vec3(0.0, 1.0, 0.0)));
	}
	tangent = normalize(tangent);
	vec3 bitangent = normalize(cross(normal, tangent));
	return mat3x3(tangent, bitangent, normal);
}

float average(vec3 v)
{
	return (v.x + v.y + v.z) / 3.0;
}

void assureNotZero(inout vec3 v)
{
	if(abs(v.x) < 0.0001f)
	{
		v.x = 0.0001f;
	}
	if(abs(v.y) < 0.0001f)
	{
		v.y = 0.0001f;
	}
	if(abs(v.z) < 0.0001f)
	{
		v.z = 0.0001f;
	}
}
bool unitCubeIntersection(vec3 origin, vec3 direction,inout vec3 entry, inout vec3 exit)
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
	bool inside = origin.x <= 1.0 && origin.x >= 0.0 && origin.y <= 1.0 && origin.y >= 0.0 && origin.z <= 1.0 && origin.z >= 0.0;
	return inside || !(tMax < 0 || tMin > tMax);
}

bool inRange(vec3 v, vec3 minV, vec3 maxV)
{
	return v.x >= minV.x && v.x <= maxV.x && v.y >= minV.y && v.y <= maxV.y && v.z >= minV.z && v.z <= maxV.z;
}

bool inUnitCube(vec3 v)
{
	return v.x >= 0.0 && v.x <= 1.0 && v.y >= 0.0 && v.y <= 1.0 && v.z >= 0.0 && v.z <= 1.0;
}


uint flatten(uvec3 idx, uvec3 size)
{
	return  idx.x + idx.y*size.x + idx.z*size.x*size.y;
}

uint flatten(uvec2 idx, uvec2 size)
{
	return  idx.x + idx.y*size.x;
}

uint flatten(uvec3 idx, uint size)
{
	return  idx.x + idx.y*size + idx.z*size*size;
}

uint flatten(uvec2 idx, uint size)
{
	return  idx.x + idx.y*size;
}

uint gridIdx(vec3 pos, uint gridSize)
{
	vec3 clampedPos = clamp(pos, 0.0, 0.9999);
	uvec3 indexVec = uvec3(clampedPos * vec3(gridSize));
	return flatten(indexVec, gridSize);
}

vec2 gridPos2D(uint idx, uint gridSize)
{
	uint x = idx % gridSize;
	uint y = idx / gridSize;
	return vec2(x, y) / float(gridSize);
}

vec2 gridPos2D(uint idx, uvec2 gridSize)
{
	uint x = idx % gridSize.x;
	uint y = idx / gridSize.x;
	return vec2(x, y) / vec2(gridSize);
}

uint gridIdx(vec2 pos, uint gridSize)
{
	vec2 clampedPos = clamp(pos, 0.0, 0.9999);
	uvec2 indexVec = uvec2(clampedPos * vec2(gridSize));
	return flatten(indexVec, gridSize);
}

uint gridIdx(vec2 pos, uvec2 gridSize)
{
	vec2 clampedPos = clamp(pos, 0.0, 0.9999);
	uvec2 indexVec = uvec2(clampedPos * vec2(gridSize));
	return flatten(indexVec, gridSize);
}

vec3 gridPos3D(uint idx, uint gridSize)
{
	uint x = idx % gridSize;
	uint y = (idx / gridSize) % gridSize;
	uint z = idx / (gridSize * gridSize);
	return vec3(x, y, z) / float(gridSize);
}



//vec3 sphericalToCartesian(float phi, float theta)
//{
//	return vec3(sin(phi)*cos(theta), sin(phi)*sin(theta), cos(phi));
//}


vec3 sphericalToCartesian(float phi, float theta)
{
	return vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
}

//vec3 sphericalToCartesian(vec2 v)
//{
//	return vec3(sin(v.y)*cos(v.x), sin(v.y)*sin(v.x), cos(v.y));
//}
vec3 sphericalToCartesian(vec2 v)
{
	return vec3(sin(v.y)*cos(v.x), sin(v.y)*sin(v.x), cos(v.y));
}


// civ
vec2 cartesianToSpherical(vec3 v)
{
	return vec2(atan(v.y, v.x), acos(v.z)); // theta, phi
	//return vec2(atan(v.y, v.x), atan(length(v.x+v.y),v.z));
	//return vec2(atan(v.y / v.x), acos(v.z));
}


vec2 invertSpherical(vec2 spherical)
{
	return vec2(mod(spherical.x + PI, 2.0*PI), PI - spherical.y);
}

vec2 normalizeSpherical(vec2 spherical)
{
	float phi = mod(spherical.x + 2.0*PI, 2.0*PI);
	float theta = mod(spherical.y + 2.0*PI, 2.0*PI);
	if(theta > PI)
	{
		theta = 2.0*PI - theta;
		phi = mod(phi + PI, 2.0*PI);
	}
	return vec2(phi, theta);
}



vec2 projectRaySegment(vec3 targetBegin, vec3 targetEnd, vec3 srcBegin, vec3 srcEnd)
{
	vec2 sampleLocation;
	vec3 dirTarget = normalize(targetEnd - targetBegin);
	float distTarget = distance(targetEnd, targetBegin);
	vec3 deltaBeginBegin = srcBegin-targetBegin;
	sampleLocation.x = dot(deltaBeginBegin, dirTarget)/distTarget;
	vec3 deltaEndBegin = srcEnd-targetBegin;
	sampleLocation.y = dot(deltaEndBegin, dirTarget)/distTarget;
	return sampleLocation;

}

float triangleArea(vec3 a, vec3 b, vec3 c)
{
	vec3 ab = b - a;
	vec3 ac = c - a;
	return 0.5 * length(cross(ab, ac));
}

float distanceSquared(vec3 a, vec3 b)
{
	vec3 d = a - b;
	return dot(d, d);
}

float absDot(vec3 a, vec3 b)
{
	return abs(dot(a, b));
}

vec2 dirToUv(vec3 v)
{
	const vec2 invAtan = vec2(0.1591, 0.3183);
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

vec3 uvToDir(vec2 uv)
{
   uv+=0.5;
   float theta = uv.y * 3.14159265359;
   float phi = uv.x * 3.14159265359 * 2.0;
   return vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
}

float pos_dot(vec3 a, vec3 b)
{
	return max(0.0, dot(a, b));
}

#endif