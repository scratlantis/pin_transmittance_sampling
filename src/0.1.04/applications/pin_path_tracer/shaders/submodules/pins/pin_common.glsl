#ifndef PIN_COMMON_H
#define PIN_COMMON_H


#ifndef PIN_POS_GRID_SIZE
#define PIN_POS_GRID_SIZE 1
#endif

#ifndef PIN_DIR_GRID_SIZE
#define PIN_DIR_GRID_SIZE 1
#endif


uint pin_cache_offset(vec3 pos, vec3 dir)
{
	uint posOffset = gridIndex(pos, PIN_POS_GRID_SIZE);
	vec2 sphericalCoords = cartesianToSpherical(dir);
	sphericalCoords = normalizeSpherical(sphericalCoords);
	if(sphericalCoords.y > PI * 0.5)
	{
		sphericalCoords = invertSpherical(sphericalCoords);
	}
	vec2 dirGridCoords;
	dirGridCoords.y = sphericalCoords.y * 0.5 * INV_PI;
	dirGridCoords.x = sin(PI*0.5 - sphericalCoords.x);
	uint dirOffset = gridIndex(dirGridCoords, PIN_DIR_GRID_SIZE);
	return posOffset * PIN_DIR_GRID_SIZE * PIN_DIR_GRID_SIZE + dirOffset;
}

void writeToMask(uint idx, inout GLSLPinCacheEntry pin)
{
	uint dataWordIdx = idx/32;
	uint bitIdx = idx%32;
	pin.mask[dataWordIdx] |= 1 << bitIdx;
}

#endif