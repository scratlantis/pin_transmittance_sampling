#include "EventManager.h"
#include "utility.h"
#include "ui.h"

void EventManager::init(FixedCamera *pCam)
{
	this->pCam = pCam;
}

void EventManager::reset()
{
	viewHasChanged  = true;
	debugView       = false;
	viewType        = 0;
	ptSplittCoef    = 0.5f;
	ptReset         = true;
	pathViewCreated = false;
	*pCam             = FixedCamera(loadCamState());
}

void EventManager::newFrame()
{
	frameCounter++;

	// Build GUI and check gui focus
	gVarsPreGui.clear();
	for (size_t i = 0; i < gVars.size(); i++)
	{
		gVarsPreGui.push_back(*gVars[i]);
	}
	buildGui();
	guiFocus = false;
	for (size_t i = 0; i < gVars.size(); i++)
	{
		guiFocus = guiFocus || !gVars[i]->compareValue(gVarsPreGui[i]);
	}
	if (guiFocus)
	{
		gState.io.clearEvents();
	}

	// Check for changes since last frame
	if (frameCounter > 1)
	{
		VKA_ASSERT(gVars.size() == gVarsLastFrame.size());
		for (uint32_t i = 0; i < gVars.size(); i++)
		{
			gVarHasChanged[*gVars[i]] = !gVars[i]->compareValue(gVarsLastFrame[i]);
		}
	}
	gVarsLastFrame.clear();
	for (size_t i = 0; i < gVars.size(); i++)
	{
		gVarsLastFrame.push_back(*gVars[i]);
	}

	// Check for key events
	if (gState.io.keyPressedEvent[GLFW_KEY_R])        // Reload shaders
	{
		clearShaderCache();
	}
	if (gState.io.keyPressedEvent[GLFW_KEY_T])        // Reset camera
	{
		*pCam = FixedCamera(DefaultFixedCameraState());
	}
	updateView();
	updatePathTraceParams();

	// Load new config
	if (gVarHasChanged[gvar_select_config])
	{
		if (gvar_select_config.val.v_uint != 0)
		{
			loadGVar(gVars,configPath + gvar_select_config.set.list[gvar_select_config.val.v_uint]);
		}
		addFileNamesToEnum(std::string(RESOURCE_BASE_DIR) + "/textures/envmap/2k/", gvar_env_map.set.list);
		gvar_select_config.set.list.clear();
		gvar_select_config.set.list.push_back("None");
		addFileNamesToEnum(configPath, gvar_select_config.set.list);
		reset();
	}

	storeCamState(pCam->getState());
	
}

bool EventManager::requestModelLoad()
{
	bool result = gVarHasChanged[gvar_model] || gVarHasChanged[gvar_env_map];
	return result;
}

void EventManager::updateView()
{
	if (mouseInView())
	{
		viewHasChanged = pCam->keyControl(0.016);
	}
	if (gState.io.mouse.rightPressed)
	{
		viewHasChanged = viewHasChanged || pCam->mouseControl(0.016);
	}
	if (gState.io.mouse.middleEvent && gState.io.mouse.middlePressed)
	{
		debugView = !debugView;
	};
	if (gState.io.keyPressedEvent[GLFW_KEY_E])
	{
		viewType = (viewType + 1) % viewTypeCount;
	}
	if (gState.io.keyPressedEvent[GLFW_KEY_Q])
	{
		viewType = (viewTypeCount + viewType - 1) % viewTypeCount;
	}
}

void EventManager::updatePathTraceParams()
{
	// Reset accumulation
	// clang-format off
	ptReset = gState.io.keyPressedEvent[GLFW_KEY_LEFT_CONTROL]
		|| gState.io.keyPressed[GLFW_KEY_LEFT_CONTROL] && gState.io.mouse.leftEvent
		|| gvar_fixed_seed.val.v_bool
		|| frameCounter <= 2
		|| viewHasChanged
		|| (gState.io.mouse.rightPressed && gState.io.mouse.leftPressed && !mouseInView())
		|| guiFocus;
	// clang-format on

	if (gState.io.keyPressedEvent[GLFW_KEY_LEFT_CONTROL])
	{
		gvar_screen_cursor_enable.val.v_bool = false;
		pathViewCreated                      = false;
	}
	if (gState.io.mouse.leftPressed && mouseInView())
	{
		if (gState.io.keyPressed[GLFW_KEY_LEFT_CONTROL])
		{
			gvar_screen_cursor_enable.val.v_bool = true;
			glm::vec2 coord             = mouseViewCoord();
			gvar_screen_cursor_pos.val.v_vec3[0] = coord.x;
			gvar_screen_cursor_pos.val.v_vec3[1] = coord.y;
		}
		else
		{
			ptSplittCoef = glm::clamp<float>(ptSplittCoef + mouseViewChange().x, 0.0, 1.0);
		}
	}
	if (gvar_screen_cursor_enable.val.v_bool && !pathViewCreated)
	{
		pathViewCreated = true;
		gvar_path_sampling_event.val.v_bool = true;
	}
	else
	{
		gvar_path_sampling_event.val.v_bool = false;
	}
}


