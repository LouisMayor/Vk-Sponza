#pragma once

#include "Logger.h"
#include "Vk-Generator/VkGenerator.hpp"
#include "InputManager.h"
#include "Camera.h"

class VkApp
{
public:

	VkApp();

	void Start();

	void Update(float);

	void Close();

	bool ShouldStop();

	void SetWindowTitle(std::string);

	Camera* ActiveCamera() const
	{
		return m_active_camera;
	}

	static VkApp* Instance();

private:

	void UpdateWindowTitle();

	static void WindowCloseCallback(GLFWwindow*);

	bool Input();

	void ToggleCursor();

	static bool m_force_close;

	InputManager m_input_manager;

	Camera::CameraInfo player_fps_camera_config
	{
		0.1f,
		10000.0f,
		70.0f,
		glm::vec3(0.331336, 0.000000, -4.927723),
		glm::vec3(0.0f),
		ECameraType::FPS
	};

	Camera  m_camera;
	Camera* m_active_camera = nullptr;

	static VkApp* m_instance;

	float m_total_time = 0.0f;
	float m_last_delta;

	std::string m_window_title = "";
};
