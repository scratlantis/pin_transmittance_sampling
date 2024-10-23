#ifndef PIN_SMD_H
#define PIN_SMD_H

layout(binding = PIN_SMD_BINDING_OFFSET) readonly buffer PIN_GRID
{
	GLSLPinCacheEntry pin_grid;
};


#endif