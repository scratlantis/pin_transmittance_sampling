#version 460
#extension GL_GOOGLE_include_directive : enable
#define PI 3.14159265359
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

struct PushDescriptor
{
		mat4 viewMat;
		mat4 inverseViewMat;
		mat4 projectionMat;
		mat4 inverseProjectionMat;
};
layout(binding = 0) uniform PUSH_DESC {PushDescriptor pd;};
layout(binding = 1) uniform sampler envMapSampler;
layout(binding = 2) uniform texture2D envMap;



vec2 SampleSphericalMap(vec3 v)
{
	const vec2 invAtan = vec2(0.1591, 0.3183);
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}



void main()
{
	outColor = vec4(1.0, 0.0, 0.0, 1.0);
    vec2 device_coord     = inUV * 2.0 - 1.0;
	vec4 clipCoords = vec4(device_coord.x, device_coord.y, 1.0, 1.0);
    vec4 target         = pd.inverseProjectionMat * clipCoords;
    vec3 direction      = (pd.inverseViewMat * target).xyz;
	direction =  normalize(direction);
	vec2 texCoords = SampleSphericalMap(direction);
	vec4 texColor = texture(sampler2D(envMap, envMapSampler), texCoords);
	outColor.rgb = texColor.rgb;
	//outColor.rg = texCoords;
}