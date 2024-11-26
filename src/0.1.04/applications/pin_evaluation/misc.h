#pragma once
#include "config.h"

FixedCameraState loadCamState();

void saveCamState(FixedCameraState state);

bool dirExists(const char *path);

bool processLoadStoreFile();

void processTraceParams();
