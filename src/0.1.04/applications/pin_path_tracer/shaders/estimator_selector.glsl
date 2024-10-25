#ifndef ESTMATOR_SELECTOR_H
#define ESTMATOR_SELECTOR_H

#ifdef PIN_SMD_H
#define SAMPLE_DISTANCE(origin, direction, maxLength, seed) pinSampleDistance(origin, direction, maxLength, seed)
#else
#define SAMPLE_DISTANCE(origin, direction, maxLength, seed) rayMarcheMedium(origin, direction, maxLength, seed)
#endif

#endif