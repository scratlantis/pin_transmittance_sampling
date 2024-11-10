#ifndef PIN_INTERFACE_STRUCTS_H
#define PIN_INTERFACE_STRUCTS_H

struct GLSLPinCacheEntryV1
{
	vec2 transmittance; // +/-
	vec2 collisionDistance; // +/-
};

struct GLSLPinCacheEntryV2
{
	uint mask;
};

struct GLSLPinCacheEntryV3
{
	float minTransmittance;
	uint mask;
};