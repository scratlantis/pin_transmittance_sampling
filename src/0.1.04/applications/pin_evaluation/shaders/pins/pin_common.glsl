#ifndef PIN_COMMON_H
#define PIN_COMMON_H


#ifndef PIN_POS_GRID_SIZE
#define PIN_POS_GRID_SIZE 1
#endif

#ifndef PIN_DIR_GRID_SIZE
#define PIN_DIR_GRID_SIZE 1
#endif


#ifndef PIN_TYPE
#define PIN_TYPE 0
#endif

#if PIN_TYPE == 1
#define PIN_STRUCT GLSLPinCacheEntryV1
#elif PIN_TYPE == 2
#define PIN_STRUCT GLSLPinCacheEntryV2
#elif PIN_TYPE == 3
#define PIN_STRUCT GLSLPinCacheEntryV3
#endif

#include "../../lib/lib/math.glsl"


#define PIN_DIR_GRID_SIZE_THETA PIN_DIR_GRID_SIZE
#define PIN_DIR_GRID_SIZE_PHI (PIN_DIR_GRID_SIZE*4)

uint pin_cache_offset(vec3 pos, vec3 dir)
{
	uint posOffset = gridIdx(pos, PIN_POS_GRID_SIZE);
	dir = normalize(dir);
	vec2 sphericalCoords = cartesianToSpherical(dir);
	if(sphericalCoords.x < 0.0)
	{
		sphericalCoords.x += PI*2.0;
	}
	if(dir.z < 0.0)
	{
		sphericalCoords = invertSpherical(sphericalCoords);
	}
	vec2 dirGridCoords;
	dirGridCoords.x = sphericalCoords.x * INV_PI*0.5;
	//dirGridCoords.y = sin(PI*0.5 - sphericalCoords.y);
	dirGridCoords.y = sphericalCoords.y * INV_PI * 2.0;
	//uint dirOffset = gridIdx(dirGridCoords, uvec2(PIN_DIR_GRID_SIZE_PHI, PIN_DIR_GRID_SIZE_THETA));
	uint dirOffset = gridIdx(dirGridCoords, uvec2(PIN_DIR_GRID_SIZE_THETA, PIN_DIR_GRID_SIZE_PHI));

	return posOffset * PIN_DIR_GRID_SIZE_THETA * PIN_DIR_GRID_SIZE_PHI + dirOffset;
}

vec3 pin_dir(uint idx)
{
	uint dirIdx = idx % (PIN_DIR_GRID_SIZE_PHI * PIN_DIR_GRID_SIZE_THETA);
	vec2 dirGridCoords = gridPos2D(dirIdx, uvec2(PIN_DIR_GRID_SIZE_PHI, PIN_DIR_GRID_SIZE_THETA));
	vec2 sphericalCoords;
	sphericalCoords.x = dirGridCoords.x * PI*2.0;
	//sphericalCoords.y = PI*0.5 - asin(dirGridCoords.y);
	sphericalCoords.y = dirGridCoords.y * PI * 0.5;
	return sphericalToCartesian(sphericalCoords);
}


vec3 pin_pos(uint idx)
{
	uint posIdx = idx / (PIN_DIR_GRID_SIZE_PHI * PIN_DIR_GRID_SIZE_THETA);
	return gridPos3D(posIdx, PIN_POS_GRID_SIZE);
}

// civ
void cell_range(uint idx, out vec3 posMin, out vec3 posMax, out vec2 angleMin, out vec2 angleMax)
{
	uint posIdx = idx / (PIN_DIR_GRID_SIZE_PHI * PIN_DIR_GRID_SIZE_THETA);
	posMin = gridPos3D(posIdx, PIN_POS_GRID_SIZE);
	posMax = gridPos3D(posIdx, PIN_POS_GRID_SIZE) + vec3(1.0/float(PIN_POS_GRID_SIZE));
	
	uint dirIdx = idx % (PIN_DIR_GRID_SIZE_PHI * PIN_DIR_GRID_SIZE_THETA);
	vec2 dirGridCoords = gridPos2D(dirIdx, uvec2(PIN_DIR_GRID_SIZE_PHI, PIN_DIR_GRID_SIZE_THETA));

	float phiMin = dirGridCoords.x * PI*2.0; // phi
	float phiMax = phiMin + PI*2.0 * (1.0 / float(PIN_DIR_GRID_SIZE_PHI));

	float thetaMin = dirGridCoords.y * PI * 0.5;
	float thetaMax = thetaMin + PI * 0.5 * (1.0 / float(PIN_DIR_GRID_SIZE_THETA));
	//float thetaMin = PI*0.5 - asin(clamp(dirGridCoords.y + 1.0/float(PIN_DIR_GRID_SIZE_THETA),0.0,0.9999) );
	//float thetaMax = PI*0.5 - asin(dirGridCoords.y);


	angleMin = vec2(phiMin, thetaMin);
	angleMax = vec2(phiMax, thetaMax);//vec2(phiMax, thetaMax);
}

bool get_pin_segment(vec3 inOrigin, vec3 inDir, out vec3 start, out vec3 end)
{
	vec3 dir = normalize(inDir);
	if(dir.z < 0)
	{
		dir = -dir;
	}
	return unitCubeIntersection(inOrigin, dir, start, end);
}

void apply_jitter(inout vec3 pos, inout vec3 dir, float jitterPos, float jitterDir, inout uint seed)
{
	if(jitterPos > 0.0 || jitterDir > 0.0)
	{
		pos = pos + jitterPos * (vec3(0.5) - random3D(seed)) * 1.0 / PIN_POS_GRID_SIZE;
		dir = dir + jitterDir * (vec3(0.5) - random3D(seed)) * 1.0 / PIN_DIR_GRID_SIZE;
		pos = clamp(pos, vec3(0.0), vec3(1.0));
		dir = normalize(dir);
	}
}



void quantise_to_pin_grid(inout vec3 pos, inout vec3 dir)
{
	uint pinIdx = pin_cache_offset(pos, dir);
	pos = pin_pos(pinIdx);
	pos += vec3(0.0) / float(PIN_POS_GRID_SIZE);
	vec3 newDir = pin_dir(pinIdx);
	if(dot(newDir, dir) < 0.0)
	{
		newDir = -newDir;
	}
	dir = newDir;
}

void quantise_to_pin_grid_with_jitter(inout vec3 pos, inout vec3 dir, inout uint seed)
{
	uint pinIdx = pin_cache_offset(pos, dir);

	vec3 posMin, posMax;
	vec2 angleMin, angleMax;
	cell_range(pinIdx, posMin, posMax, angleMin, angleMax);
	pos = mix(posMin, posMax, random3D(seed + 0x491630));

	vec2 angle = mix(angleMin, angleMax, random2D(seed + 0x362123));
	vec3 newDir = sphericalToCartesian(angle);

	if(dot(newDir, dir) < 0.0)
	{
		newDir = -newDir;
	}
	dir = newDir;
}

#endif