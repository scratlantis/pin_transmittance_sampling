#ifndef PIN_COMMON_H
#define PIN_COMMON_H


#ifndef PIN_POS_GRID_SIZE
#define PIN_POS_GRID_SIZE 1
#endif

#ifndef PIN_DIR_GRID_SIZE
#define PIN_DIR_GRID_SIZE 1
#endif




#include "../../lib/lib/math.glsl"

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

void cell_range(uint idx, out vec3 posMin, out vec3 posMax, out vec2 angleMin, out vec2 angleMax)
{
	uint posIdx = idx / (PIN_DIR_GRID_SIZE * PIN_DIR_GRID_SIZE);
	posMin = gridPos3D(posIdx, PIN_POS_GRID_SIZE);
	posMax = posMin + vec3(1.0/PIN_POS_GRID_SIZE);
	
	uint dirIdx = idx % (PIN_DIR_GRID_SIZE * PIN_DIR_GRID_SIZE);
	vec2 dirGridCoords = gridPos2D(dirIdx, PIN_DIR_GRID_SIZE);

	float phiMin = dirGridCoords.x * PI*2.0; // phi
	float phiMax = phiMin + PI*2.0 * (1.0 / float(PIN_DIR_GRID_SIZE));

	float thetaMin = PI*0.5 - asin(dirGridCoords.y + 1.0/float(PIN_DIR_GRID_SIZE) );
	float thetaMax = PI*0.5 - asin(dirGridCoords.y);


	angleMin = vec2(phiMin, thetaMin);
	angleMax = vec2(phiMax, thetaMax);
}

bool get_pin_segment(vec3 inOrigin, vec3 inDir, out vec3 start, out vec3 end)
{
	vec3 dir = normalize(inDir);
	if(dir.z < 0)
	{
		dir = -dir;
	}
	vec3 start, end;
	return unitCubeIntersection(origin, dir, start, end);
}

#endif