#include "misc.h"
#include "ui.h"
// Camera
GVar gvar_camera_fixpoint{"Camera Fixpoint", {0.0f, 0.0f, 0.0f}, GVAR_VEC3, GUI_CAT_CAM};
GVar gvar_camera_distance{"Camera Distance", 1.0f, GVAR_FLOAT_RANGE, GUI_CAT_CAM, {0.1f, 100.0f}};
GVar gvar_camera_up{"Camera Up", {0.0f, 1.0f, 0.0f}, GVAR_VEC3, GUI_CAT_CAM};
GVar gvar_camera_yaw{"Camera Yaw", 90.f, GVAR_FLOAT_RANGE, GUI_CAT_CAM, {0.0f, 360.0f}};
GVar gvar_camera_pitch{"Camera Pitch", 0.0f, GVAR_FLOAT_RANGE, GUI_CAT_CAM, {-90.0f, 90.0f}};
GVar gvar_camera_move_speed{"Camera Move Speed", 2.0f, GVAR_FLOAT_RANGE, GUI_CAT_CAM, {0.1f, 10.0f}};
GVar gvar_camera_turn_speed{"Camera Turn Speed", 0.25f, GVAR_FLOAT_RANGE, GUI_CAT_CAM, {0.01f, 1.0f}};
GVar gvar_camera_scroll_speed{"Camera Scroll Speed", 0.1f, GVAR_FLOAT_RANGE, GUI_CAT_CAM, {0.01f, 1.0f}};
GVar gvar_camera_reset{"Camera Reset", false, GVAR_EVENT, GUI_CAT_CAM};

FixedCameraState loadCamState()
{
	FixedCameraState state{};
	if (gvar_camera_reset.val.v_bool)
	{
		state = DefaultFixedCameraState();
	}
	else
	{
		state.fixpoint    = gvar_camera_fixpoint.val.getVec3();
		state.distance    = gvar_camera_distance.val.v_float;
		state.up          = gvar_camera_up.val.getVec3();
		state.yaw         = gvar_camera_yaw.val.v_float;
		state.pitch       = gvar_camera_pitch.val.v_float;
		state.moveSpeed   = gvar_camera_move_speed.val.v_float;
		state.turnSpeed   = gvar_camera_turn_speed.val.v_float;
		state.scrollSpeed = gvar_camera_scroll_speed.val.v_float;
	}
	return state;
}

void saveCamState(FixedCameraState state)
{
	gvar_camera_fixpoint.val.setVec3(state.fixpoint);
	gvar_camera_distance.val.v_float = state.distance;
	gvar_camera_up.val.setVec3(state.up);
	gvar_camera_yaw.val.v_float          = state.yaw;
	gvar_camera_pitch.val.v_float        = state.pitch;
	gvar_camera_move_speed.val.v_float   = state.moveSpeed;
	gvar_camera_turn_speed.val.v_float   = state.turnSpeed;
	gvar_camera_scroll_speed.val.v_float = state.scrollSpeed;
}


// Files
// GVar gvar_file_path{"File Path", std::string("none"), GVAR_DISPLAY_TEXT, GUI_CAT_FILE_LOAD};
GVar gvar_load_file{"Load file", std::string("none"), GVAR_FILE_INPUT, GUI_CAT_FILE_LOAD, std::vector<std::string>({".json", configPath}), GVAR_FLAGS_NO_LOAD};
GVar gvar_save_as_file{"Save as", std::string("none"), GVAR_FILE_OUTPUT, GUI_CAT_FILE_SAVE, std::vector<std::string>({".json", configPath})};
GVar gvar_save_file{"Save file", false, GVAR_EVENT, GUI_CAT_FILE_SAVE};

bool processLoadStoreFile()
{
	// Load
	if (gvar_load_file.val.v_bool)
	{
		if (std::filesystem::exists(gvar_load_file.val.v_char_array.data()))
		{
			GVar::loadAll(gvar_load_file.val.v_char_array.data());
			return true;
		}
	}
	// Save
	if (gvar_save_file.val.v_bool || gvar_save_as_file.val.v_bool)
	{
		std::string dir = gvar_save_as_file.val.v_char_array.data();
		dir = dir.substr(0, dir.find_last_of("\\"));
		if (std::filesystem::exists(dir))
		{
			GVar::storeAll(gvar_save_as_file.val.v_char_array.data());
		}
	}
	return false;
}