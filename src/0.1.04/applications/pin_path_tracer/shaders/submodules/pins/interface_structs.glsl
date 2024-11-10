#ifndef PIN_INTERFACE_STRUCTS_H
#define PIN_INTERFACE_STRUCTS_H

#ifndef PIN_MASK_SIZE
#define PIN_MASK_SIZE 1
#endif
struct GLSLPinCacheEntry
{
	float maxColProb; //Maximum Collision probability per quantisation step
	float transmittancePos;
	float transmittanceNeg;
	uint mask[PIN_MASK_SIZE];
};

#endif