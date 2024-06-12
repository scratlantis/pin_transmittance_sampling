#pragma once
#include "types.h"
#include <vka/resource_objects/resource_common.h>
#include <vka/state_objects/global_state.h>
#include "cmd_buffer_functionality.h"

void vkaSwapBuffers(std::vector<VkaCommandBuffer> cmdBufs);
