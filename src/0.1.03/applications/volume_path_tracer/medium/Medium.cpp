#include "Medium.h"
#include "config.h"

GVar gvar_medium_x{"Medium X", -0.2f, GVAR_FLOAT_RANGE, GENERAL,{-1.0f,1.0f}};
GVar gvar_medium_y{"Medium Y", -0.2f, GVAR_FLOAT_RANGE, GENERAL,{-1.0f,1.0f}};
GVar gvar_medium_z{"Medium Z", -0.2f, GVAR_FLOAT_RANGE, GENERAL,{-1.0f,1.0f}};
GVar gvar_medium_rot_y{"Medium Rotation Y Axis", 0.0f, GVAR_FLOAT_RANGE, GENERAL, {0, 360.0f}};
GVar gvar_medium_scale{"Medium Scale", 0.3f, GVAR_FLOAT_RANGE, GENERAL, {0.05f, 1.0f}};