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
	//pathViewCreated = false;
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

	if (gState.io.keyPressed[GLFW_KEY_LEFT_CONTROL] && gState.io.keyPressedEvent[GLFW_KEY_S])
	{
		gvar_save_config.val.v_bool = true;
		std::string name = gvar_select_config.set.list[gvar_select_config.val.v_uint];
		name			 = name.substr(0, name.find_last_of("."));
		gvar_save_config_name.val  = GVar_Val(name);
	}

	if (gState.io.keyPressed[GLFW_KEY_LEFT_CONTROL] && (gState.io.keyPressedEvent[GLFW_KEY_Z] || gState.io.keyPressedEvent[GLFW_KEY_Y]))
	{
		gvar_reload_config.val.v_bool = true;
	}

	if (gvar_save_config.val.v_bool)
	{
		storeGVar(gVars, configPath + std::string(gvar_save_config_name.val.v_char_array.data() + std::string(".json")));
	}

	if (gvar_save_config.val.v_bool || frameCounter < 2)
	{
		gvar_select_config.set.list.clear();
		gvar_select_config.set.list.push_back("None");
		addFileNamesToEnum(configPath, gvar_select_config.set.list);
		for (uint32_t i = 0; i < gvar_select_config.set.list.size(); i++)
		{
			std::string name = gvar_save_config_name.val.v_char_array.data();
			name += ".json";
			if (gvar_select_config.set.list[i] == name)
			{
				gvar_select_config.val.v_uint = i;
			}
		}
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
	

	
	// Load new config
	
	if (!gvar_save_config.val.v_bool && gVarHasChanged[gvar_select_config] || gvar_reload_config.val.v_bool)
	{
		if (gvar_select_config.val.v_uint != 0)
		{
			loadGVar(gVars,configPath + gvar_select_config.set.list[gvar_select_config.val.v_uint]);
			for (uint32_t i = 0; i < gvar_select_config.set.list.size(); i++)
			{
				std::string name = gvar_select_config.set.list[gvar_select_config.val.v_uint];
				if (gvar_select_config.set.list[i] == name)
				{
					gvar_select_config.val.v_uint = i;
				}
			}
		}
		gvar_env_map.set.list.clear();
		addFileNamesToEnum(std::string(RESOURCE_BASE_DIR) + "/textures/envmap/2k/", gvar_env_map.set.list);
		reset();
	}

	float moveCoef = 0.2f;
	float scaleCoef = 0.04f;
	if (gState.io.keyPressed[GLFW_KEY_X])
	{
		gvar_medium_pos.val.v_vec3[0] += gState.io.mouse.scrollChange * moveCoef;
		gvar_medium_pos.val.v_vec3[0] = glm::clamp(gvar_medium_pos.val.v_vec3[0], gvar_medium_pos.set.range.min.v_float, gvar_medium_pos.set.range.max.v_float);
		viewHasChanged                = true;
	}
	else if (gState.io.keyPressed[GLFW_KEY_C])
	{
		gvar_medium_pos.val.v_vec3[1] += gState.io.mouse.scrollChange * moveCoef;
		gvar_medium_pos.val.v_vec3[1] = glm::clamp(gvar_medium_pos.val.v_vec3[1], gvar_medium_pos.set.range.min.v_float, gvar_medium_pos.set.range.max.v_float);
		viewHasChanged                = true;
	}
	else if (gState.io.keyPressed[GLFW_KEY_V])
	{
		gvar_medium_pos.val.v_vec3[2] += gState.io.mouse.scrollChange * moveCoef;
		gvar_medium_pos.val.v_vec3[2] = glm::clamp(gvar_medium_pos.val.v_vec3[2], gvar_medium_pos.set.range.min.v_float, gvar_medium_pos.set.range.max.v_float);
		viewHasChanged                = true;
	}
	else if (gState.io.keyPressed[GLFW_KEY_B])
	{
		gvar_medium_scale.val.v_float += gState.io.mouse.scrollChange * scaleCoef;
		gvar_medium_scale.val.v_float = glm::clamp(gvar_medium_scale.val.v_float, gvar_medium_scale.set.range.min.v_float, gvar_medium_scale.set.range.max.v_float);
		viewHasChanged                = true;
	}
	else
	{
		updateView();
	}
	updatePathTraceParams();

	storeCamState(pCam->getState());
	
}

bool EventManager::requestModelLoad()
{
	bool result = gVarHasChanged[gvar_model] || gVarHasChanged[gvar_env_map];
	return result;
}

void EventManager::updateView()
{
	if (!gState.io.keyPressed[GLFW_KEY_LEFT_CONTROL])
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
		if (debugView)
		{
			if (gState.io.keyPressedEvent[GLFW_KEY_E])
			{
				debugViewType = (debugViewType + 1) % debugViewTypeCount;
			}
			if (gState.io.keyPressedEvent[GLFW_KEY_Q])
			{
				debugViewType = (debugViewTypeCount + debugViewType - 1) % debugViewTypeCount;
			}
		}
		else
		{
			if (gState.io.keyPressedEvent[GLFW_KEY_E])
			{
				viewType = (viewType + 1) % viewTypeCount;
			}
			if (gState.io.keyPressedEvent[GLFW_KEY_Q])
			{
				viewType = (viewTypeCount + viewType - 1) % viewTypeCount;
			}
		}
	}
}

void EventManager::updatePathTraceParams()
{
	// Reset accumulation
	// clang-format off
	ptReset = gState.io.keyPressed[GLFW_KEY_LEFT_CONTROL] && gState.io.mouse.rightEvent
		|| gState.io.keyPressed[GLFW_KEY_LEFT_CONTROL] && gState.io.mouse.leftEvent
		|| gvar_fixed_seed.val.v_bool
		|| frameCounter <= 2
		|| viewHasChanged
		|| (gState.io.mouse.rightPressed && gState.io.mouse.leftPressed && !mouseInView())
		|| guiFocus;
	// clang-format on

	if (gvar_reload_config.val.v_bool)
	{
		ptReset = true;
		pathViewCreated = false;
	}
	if (gState.io.keyPressed[GLFW_KEY_LEFT_CONTROL] && gState.io.mouse.rightEvent)
	{
		gvar_screen_cursor_enable.val.v_bool = false;
		pathViewCreated                      = false;
	}
	if (gState.io.mouse.leftPressed && mouseInView())
	{
		if (gState.io.keyPressed[GLFW_KEY_LEFT_CONTROL])
		{
			if (gState.io.mouse.leftEvent)
			{
				gvar_screen_cursor_enable.val.v_bool = true;
				pathViewCreated                      = false;
				glm::vec2 coord             = mouseViewCoord();
				gvar_screen_cursor_pos.val.v_vec3[0] = coord.x;
				gvar_screen_cursor_pos.val.v_vec3[1] = coord.y;
				gvar_screen_cursor_seed.val.v_uint   = frameCounter;
			}
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


