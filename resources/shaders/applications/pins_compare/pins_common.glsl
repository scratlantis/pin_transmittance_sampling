#define MAX_RAY_DISTANCE 10000.0
#define MIN_RAY_DISTANCE 0.1

#define EPSILON 0.0001
#ifndef GAUSSIAN_COUNT
	#define GAUSSIAN_COUNT 10
#endif

#ifndef PIN_GRID_SIZE
	#define PIN_GRID_SIZE 10
#endif

#ifndef PINS_PER_GRID_CELL
	#define PINS_PER_GRID_CELL 10
#endif

#ifndef PIN_COUNT
	#define PIN_COUNT 10
#endif

#ifndef PIN_COUNT_SQRT
	#define PIN_COUNT_SQRT 10
#endif

#define GAUS_COEF 0.2
#define PI 3.14159265359

struct Gaussian
{
	vec3  mean;
	float variance;
};

struct Pin
{
	vec2 theta;
	vec2 phi;
};


struct PinData
{
	uint pinIndex;
	uint padding[3];
};

struct PinGridEntry
{
	Pin pin;
	PinData data;
};

struct Cube
{
	mat4 modelMatrix;
	mat4 invModelMatrix;
};

struct View
{
	mat4 viewMat;
	mat4 inverseViewMat;
	mat4 projectionMat;
	mat4 inverseProjectionMat;
	mat4 rotationMatrix;

	vec4 camPos;

	uint width;
	uint height;
	uint frameCounter;
	uint usePins;

	Cube cube;
};

struct RaySegment
{
	vec3 origin;
	vec3 direction;
	float tMin;
	float tMax;
};


void getPoints(RaySegment seg, inout vec3 p1, inout vec3 p2)
{
	p1 = seg.origin + seg.direction * seg.tMin;
	p2 = seg.origin + seg.direction * seg.tMax;
}

void getRaySegment(inout RaySegment seg, vec3 p1, vec3 p2)
{
	seg.origin = p1;
	seg.direction = normalize(p2 - p1);
	seg.tMin = 0.0;
	seg.tMax = length(p2 - p1);
}

RaySegment transform(RaySegment seg, mat4 m)
{
	RaySegment outSeg = seg;
	outSeg.origin = (m * vec4(seg.origin, 1)).xyz;
	outSeg.direction = (m * vec4(seg.direction, 0)).xyz;
	float invScale = length(seg.direction / outSeg.direction);
	outSeg.tMin = seg.tMin * invScale;
	outSeg.tMax = seg.tMax * invScale;
	return outSeg;

}

void assureNotZero(inout vec3 v)
{
	if(abs(v.x) < EPSILON)
	{
		v.x = EPSILON;
	}
	if(abs(v.y) < EPSILON)
	{
		v.y = EPSILON;
	}
	if(abs(v.z) < EPSILON)
	{
		v.z = EPSILON;
	}
}

bool intersectCube(inout RaySegment raySegOut, RaySegment raySegIn, Cube cube)
{
	raySegOut = transform(raySegIn, cube.invModelMatrix);
	assureNotZero(raySegOut.direction);
	vec3 invDir = 1.0 / raySegOut.direction;
	vec3 c1 = -raySegOut.origin*invDir;
	vec3 c2 = c1 + invDir; // (vec(1.0)-raySeg.origin)*invDir;
	raySegOut.tMin = max(max(min(c1.x, c2.x), min(c1.y, c2.y)), min(c1.z, c2.z));
	raySegOut.tMax = min(min(max(c1.x, c2.x), max(c1.y, c2.y)), max(c1.z, c2.z)); 
	return !(raySegOut.tMax < 0 || raySegOut.tMin > raySegOut.tMax);
}

bool intersectAABB(vec3 origin, vec3 direction, vec3 boxMin, vec3 boxMax)
{
	vec3 dir = direction;
	assureNotZero(dir);
	vec3 invDir = 1.0 / dir;
	vec3 c1 = (boxMin-origin)*invDir;
	vec3 c2 = (boxMax-origin)*invDir;
	float tMin = max(max(min(c1.x, c2.x), min(c1.y, c2.y)), min(c1.z, c2.z));
	float tMax = min(min(max(c1.x, c2.x), max(c1.y, c2.y)), max(c1.z, c2.z)); 
	return !(tMax < 0 || tMin > tMax);
}

uint getCellIndex(uvec3 idx)
{
	return  (idx.x + idx.y*PIN_GRID_SIZE + idx.z*PIN_GRID_SIZE*PIN_GRID_SIZE)*PINS_PER_GRID_CELL;
}


float eval_gaussian(float x, float mean, float variance)
{
	float c = 0.3989422804014337; // 1/sqrt(2*pi)
	float a = (x - mean) / variance;
	return GAUS_COEF*(1.0/variance)*c*exp(-0.5 * a * a);
}

float evalTransmittanceGaussian(RaySegment raySegIn, Gaussian g)
{
	 float t = dot(raySegIn.direction, g.mean - raySegIn.origin);
	 vec3 p = raySegIn.origin + t * raySegIn.direction;
	 float d = length(p - g.mean);
	 float f1 = eval_gaussian(d, 0.0, g.variance);
	 return f1;
}
float evalTransmittanceGaussian(vec3 origin, vec3 direction, Gaussian g)
{
	 float t = dot(direction, g.mean - origin);
	 vec3 p = origin + t * direction;
	 float d = length(p - g.mean);
	 float f1 = eval_gaussian(d, 0.0, g.variance);
	 return f1;
}

void getRay(Pin pin, inout vec3 origin, inout vec3 direction)
{
	vec2 x = sin(pin.theta) * cos(pin.phi);
    vec2 y = sin(pin.theta) * sin(pin.phi);
    vec2 z = cos(pin.theta);
	origin = vec3(x.x, y.x, z.x)+vec3(0.5);
	origin*=0.866025403784; // sqrt(3)/2
	direction = normalize(vec3(x.y - x.x, y.y - y.x, z.y - z.x));
}




uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

