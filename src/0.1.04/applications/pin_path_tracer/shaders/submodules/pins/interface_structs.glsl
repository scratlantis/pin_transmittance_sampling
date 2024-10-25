#ifndef PIN_INTERFACE_STRUCTS_H
#define PIN_INTERFACE_STRUCTS_H

#define PIN_MASK_SIZE 1
struct GLSLPinCacheEntry
{
	float maxColProb; //Maximum Collision probability per quantisation step
	uint mask[PIN_MASK_SIZE];
};

#endif