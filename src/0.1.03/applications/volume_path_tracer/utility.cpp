#include "utility.h"
#include <filesystem>


void addFileNamesToEnum(std::string path, std::vector<std::string> &enumVal)
{
	for (const auto &entry : std::filesystem::directory_iterator(path))
	{
		std::string name = entry.path().string();
		name             = name.substr(name.find_last_of("/") + 1);
		enumVal.push_back(name);
	}
}

GVar gvar_cam_fixpoint     = {"Camera Position", {0.0, 0.0, 0.0}, GVAR_VEC3, NO_GUI};
GVar gvar_cam_distance     = {"Camera Distance", 1.0f, GVAR_FLOAT, NO_GUI};
GVar gvar_cam_up           = {"Camera Up", {0.0, 1.0, 0.0}, GVAR_VEC3, NO_GUI};
GVar gvar_cam_yaw          = {"Camera Yaw", 90.0f, GVAR_FLOAT, NO_GUI};
GVar gvar_cam_pitch        = {"Camera Pitch", 0.0f, GVAR_FLOAT, NO_GUI};
GVar gvar_cam_move_speed   = {"Camera Move Speed", 0.2f, GVAR_FLOAT, NO_GUI};
GVar gvar_cam_turn_speed   = {"Camera Turn Speed", 0.25f, GVAR_FLOAT, NO_GUI};
GVar gvar_cam_scroll_speed = {"Camera Scroll Speed", 0.1f, GVAR_FLOAT, NO_GUI};

void storeCamState(const FixedCameraState &camState)
{
	gvar_cam_fixpoint.val.v_vec3[0] = camState.fixpoint.x;
	gvar_cam_fixpoint.val.v_vec3[1] = camState.fixpoint.y;
	gvar_cam_fixpoint.val.v_vec3[2] = camState.fixpoint.z;
	gvar_cam_distance.val.v_float   = camState.distance;
	gvar_cam_up.val.v_vec3[0]       = camState.up.x;
	gvar_cam_up.val.v_vec3[1]       = camState.up.y;
	gvar_cam_up.val.v_vec3[2]       = camState.up.z;
	gvar_cam_yaw.val.v_float        = camState.yaw;
	gvar_cam_pitch.val.v_float      = camState.pitch;

	gvar_cam_move_speed.val.v_float   = camState.moveSpeed;
	gvar_cam_turn_speed.val.v_float   = camState.turnSpeed;
	gvar_cam_scroll_speed.val.v_float = camState.scrollSpeed;
}

FixedCameraState loadCamState()
{
	FixedCameraState camState{};
	camState.fixpoint.x = gvar_cam_fixpoint.val.v_vec3[0];
	camState.fixpoint.y = gvar_cam_fixpoint.val.v_vec3[1];
	camState.fixpoint.z = gvar_cam_fixpoint.val.v_vec3[2];
	camState.distance   = gvar_cam_distance.val.v_float;
	camState.up.x       = gvar_cam_up.val.v_vec3[0];
	camState.up.y       = gvar_cam_up.val.v_vec3[1];
	camState.up.z       = gvar_cam_up.val.v_vec3[2];
	camState.yaw        = gvar_cam_yaw.val.v_float;
	camState.pitch      = gvar_cam_pitch.val.v_float;

	camState.moveSpeed   = gvar_cam_move_speed.val.v_float;
	camState.turnSpeed   = gvar_cam_turn_speed.val.v_float;
	camState.scrollSpeed = gvar_cam_scroll_speed.val.v_float;
	return camState;
}
