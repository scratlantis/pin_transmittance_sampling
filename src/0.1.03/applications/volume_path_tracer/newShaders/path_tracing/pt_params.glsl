#ifndef MAX_BOUNCES
#define MAX_BOUNCES 8
//#define MAX_BOUNCES 2
#endif

#ifndef SAMPLE_COUNT
#define SAMPLE_COUNT 1
#endif

#ifndef TMIN
#define TMIN 0.0001
#endif

#ifndef TMAX
#define TMAX 1000.0
#endif

#ifndef AVERAGE_RAY_MARCHE_STEP_SIZE
#define AVERAGE_RAY_MARCHE_STEP_SIZE 0.2
#endif

#define PIN_MODE_FLAG_INDIRECT 0x1
#define PIN_MODE_FLAG_DIRECT 0x2

#ifndef MIN_PIN_BOUNCE
#define MIN_PIN_BOUNCE 1
#endif


#ifndef AREA_LIGHT_COUNT
#define AREA_LIGHT_COUNT 2
#endif

#ifndef BACKGROUND_LIGHTING
//#define BACKGROUND_LIGHTING (vec3(1.0, 0.8, 0.2) * 3.0)
#define BACKGROUND_LIGHTING (vec3(1.0, 1.0, 1.0) * 0.2)
#endif