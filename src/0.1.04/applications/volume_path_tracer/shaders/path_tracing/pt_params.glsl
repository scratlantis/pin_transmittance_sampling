#ifndef MAX_BOUNCES
#define MAX_BOUNCES 8
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

#ifndef AREA_LIGHT_COUNT
#define AREA_LIGHT_COUNT 2
#endif

// Raymarching
#ifndef RAY_MARCHE_STEP_SIZE
#define RAY_MARCHE_STEP_SIZE 0.2
#endif

// Pins
#ifndef MIN_PIN_BOUNCE
#define MIN_PIN_BOUNCE 1
#endif

#define PIN_MODE_FLAG_INDIRECT 0x1
#define PIN_MODE_FLAG_DIRECT 0x2

#ifdef USE_PINS
	#ifndef PIN_MODE
		//#define PIN_MODE PIN_MODE_FLAG_DIRECT | PIN_MODE_FLAG_INDIRECT
		#define PIN_MODE PIN_MODE_FLAG_INDIRECT
	#endif
	#else
		#undef PIN_MODE
		#define PIN_MODE 0
#endif


#ifndef PLOT_POINTS
#define PLOT_POINTS 10
#endif