#ifndef PIN_COMMON_H
#define PIN_COMMON_H


#ifndef PIN_POS_GRID_SIZE
#define PIN_POS_GRID_SIZE 1
#endif

#ifndef PIN_DIR_GRID_SIZE
#define PIN_DIR_GRID_SIZE 1
#endif

#include "../../../lib/lib/math.glsl"

uint pin_cache_offset(vec3 pos, vec3 dir)
{
	uint posOffset = gridIdx(pos, PIN_POS_GRID_SIZE);
	vec2 sphericalCoords = cartesianToSpherical(dir);
	sphericalCoords = normalizeSpherical(sphericalCoords);
	if(sphericalCoords.y > PI * 0.5)
	{
		sphericalCoords = invertSpherical(sphericalCoords);
	}
	vec2 dirGridCoords;
	dirGridCoords.x = sphericalCoords.x * INV_PI*0.5;
	dirGridCoords.y = sin(PI*0.5 - sphericalCoords.y);
	uint dirOffset = gridIdx(dirGridCoords, PIN_DIR_GRID_SIZE);
	return posOffset * PIN_DIR_GRID_SIZE * PIN_DIR_GRID_SIZE + dirOffset;
}

vec3 pin_pos(uint idx)
{
	uint posIdx = idx / (PIN_DIR_GRID_SIZE * PIN_DIR_GRID_SIZE);
	return gridPos3D(posIdx, PIN_POS_GRID_SIZE);
}

vec3 pin_dir(uint idx)
{
	uint dirIdx = idx % (PIN_DIR_GRID_SIZE * PIN_DIR_GRID_SIZE);
	vec2 dirGridCoords = gridPos2D(dirIdx, PIN_DIR_GRID_SIZE);
	vec2 sphericalCoords;
	sphericalCoords.x = dirGridCoords.x * PI*2.0;
	sphericalCoords.y = PI*0.5 - asin(dirGridCoords.y);
	return sphericalToCartesian(sphericalCoords);
}

void writeToMask(uint idx, inout GLSLPinCacheEntry pin)
{
	uint dataWordIdx = idx/32;
	uint bitIdx = idx%32;
	pin.mask[dataWordIdx] |= 1 << bitIdx;
}

void resetMask(inout GLSLPinCacheEntry pin)
{
	for(uint i = 0; i < PIN_MASK_SIZE; i++)
	{
		pin.mask[i] = 0;
	}
}

#endif