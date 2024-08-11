
struct VKAOffsetBufferEntry
{
	uint vertexOffset;
	uint indexOffset;
	uint materialOffset;
	uint padding;
};

struct VKAAreaLight
{
	// NEE select light source
	vec3  center;
	uint  padding1[1];

	vec3  normal;
	uint  padding2[1];

	vec3  v0;
	uint  padding3[1];

	vec3  v1;
	uint  padding4[1];

	vec3  v2;
	float importance; // Area * intensity

};