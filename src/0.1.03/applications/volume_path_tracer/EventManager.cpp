#include "EventManager.h"
#include "utility.h"

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
	if (gState.io.keyPressedEvent[GLFW_KEY_R])
	{
		clearShaderCache();
	}
	if (gState.io.keyPressedEvent[GLFW_KEY_T])        // Reset camera
	{
		*pCam = FixedCamera(DefaultFixedCameraState());
	}
	if (lastConfig != gvar_select_config.val.v_uint)
	{
		lastConfig = gvar_select_config.val.v_uint;
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
	updateView();
	updatePathTraceParams();
}

bool EventManager::requestModelLoad()
{
	bool result     = lastModelIndex != gvar_model.val.v_uint || lastEnvMapIndex != gvar_env_map.val.v_uint;
	lastModelIndex = gvar_model.val.v_uint;
	lastEnvMapIndex = gvar_env_map.val.v_uint;
	return result;
}

void EventManager::updateView()
{
	viewHasChanged = pCam->keyControl(0.016);
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
	ptReset = gState.io.keyPressedEvent[GLFW_KEY_LEFT_CONTROL] || gState.io.keyPressed[GLFW_KEY_LEFT_CONTROL] && gState.io.mouse.leftEvent || gvar_fixed_seed.val.v_bool || frameCounter <= 2 || viewHasChanged || gState.io.mouse.leftPressed && gState.io.mouse.leftPressed && gState.io.mouse.pos.x < 0.2 * gState.io.extent.width;


	if (gState.io.keyPressedEvent[GLFW_KEY_LEFT_CONTROL])
	{
		gvar_screen_cursor_enable.val.v_bool = false;
		pathViewCreated                      = false;
	}
	if (gState.io.mouse.leftPressed && gState.io.mouse.pos.x > 0.2 * gState.io.extent.width)
	{
		if (gState.io.keyPressed[GLFW_KEY_LEFT_CONTROL])
		{
			gvar_screen_cursor_enable.val.v_bool = true;

			gvar_screen_cursor_pos.val.v_vec3[0] = gState.io.mouse.pos.x;
			gvar_screen_cursor_pos.val.v_vec3[1] = gState.io.mouse.pos.y;
		}
		else
		{
			ptSplittCoef = glm::clamp<float>(ptSplittCoef + gState.io.mouse.change.x / (0.8f * gState.io.extent.width), 0.0, 1.0);
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


