#include "Medium.h"
#include "config.h"

GVar gvar_medium_pos{"Medium Pos", {-0.2f,-0.2f,-0.2f}, GVAR_VEC3_RANGE, MEDIUM, {-4.0f, 4.0f}};
GVar gvar_medium_rot_y{"Medium Rotation Y Axis", 0.0f, GVAR_FLOAT_RANGE, MEDIUM, {0, 360.0f}};
GVar gvar_medium_scale{"Medium Scale", 0.3f, GVAR_FLOAT_RANGE, MEDIUM, {0.05f, 4.0f}};
GVar gvar_medium_albedo{"Medium Albedo", {1.f, 1.f, 1.f}, GVAR_VEC3_RANGE, MEDIUM, {0.f, 1.f}};
