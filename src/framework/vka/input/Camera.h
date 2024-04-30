#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

// Based on Official Vulkan Tutorial
namespace vka
{
struct CameraCI
{
	glm::vec3 start_position;
	float     near_plane;
	float     far_plane;
	float     angle;
	float     fov;
	glm::vec3 start_up;
	float     start_yaw;
	float     start_pitch;
	float     start_move_speed;
	float     start_turn_speed;
	glm::vec3 directional_light_direction;
};

struct CameraCI_Default : public CameraCI
{
	CameraCI_Default()
	{
		start_position              = glm::vec3(-5.0f, 0.0f, 0.0f);
		near_plane                  = 0.1f;
		far_plane                   = 10000.f;
		angle                       = 0.0f;
		fov                         = 60.f;
		start_up                    = glm::vec3(0.0f, 1.0f, 0.0f);
		start_yaw                   = 0.f;
		start_pitch                 = 0.0f;
		start_move_speed            = 0.1f;
		start_turn_speed            = 0.25f;
		directional_light_direction = {-1.f, 1.f, 1.f};
	}
};

class Camera
{
  public:
	Camera();

	Camera(const CameraCI &ci);

	void key_control(bool *keys, GLfloat delta_time);
	void mouse_control(GLfloat x_change, GLfloat y_change);

	glm::vec3 get_camera_position();
	glm::vec3 get_camera_direction();
	glm::vec3 get_up_axis();
	glm::vec3 get_right_axis();
	glm::vec3 get_front_axis();
	GLfloat   get_near_plane();
	GLfloat   get_far_plane();
	GLfloat   get_fov();
	GLfloat   get_yaw();
	void      set_near_plane(GLfloat near_plane);
	void      set_far_plane(GLfloat far_plane);
	void      set_fov(GLfloat fov);

	void set_camera_position(glm::vec3 new_camera_position);

	glm::quat calculate_viewQuat();

	glm::mat4 calculate_viewmatrix();

	~Camera();

	GLfloat   yaw;
	GLfloat   pitch;
	glm::vec4 rotation;

  private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;

	GLfloat movement_speed;
	GLfloat turn_speed;

	GLfloat near_plane, far_plane, fov;

	void  update();
	float to_radians(float angle_in_degrees);
};
}        // namespace vka
