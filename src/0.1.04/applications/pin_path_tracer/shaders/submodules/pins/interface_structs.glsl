
#define PIN_MASK_SIZE 1
struct GLSLPinCacheEntry
{
	float maxColProb; //Maximum Collision probability per quantisation step
	uint mask[PIN_MASK_SIZE];
};