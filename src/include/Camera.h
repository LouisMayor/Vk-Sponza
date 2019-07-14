#pragma once

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

extern Logger g_Logger;

enum class ECameraType
{
	FPS,
	Free,
	Static
};

class Camera
{
public:

	struct CameraInfo
	{
		float       near_clip;
		float       far_clip;
		float       field_of_view;
		glm::vec3   position;
		glm::vec3   orientation;
		ECameraType mode;

		void operator=(const CameraInfo& _data)
		{
			near_clip     = _data.near_clip;
			far_clip      = _data.far_clip;
			position      = _data.position;
			orientation   = _data.orientation;
			mode          = _data.mode;
			field_of_view = _data.field_of_view;
		}
	};

	Camera() = default;

	void operator=(const CameraInfo& _creation_data)
	{
		m_data = _creation_data;
		UpdateCamera();
	}

	void SetProjection(const glm::vec2 _dimensions)
	{
		m_projection_matrix = glm::perspective(glm::radians(m_data.field_of_view),
		                                       _dimensions.x / _dimensions.y,
		                                       m_data.near_clip, m_data.far_clip);
		m_projection_matrix[1][1] *= -1;
	}

	void IgnoreMouse(const bool _state)
	{
		m_ignore_mouse = _state;
	}

	void Update(float _delta)
	{
		if (m_data.mode == ECameraType::Static)
		{
			return;
		}

		glm::vec3   pos      = m_data.position;
		const float movement = m_movement_speed * _delta;

		if (InputManager::Instance()->KeyHeld(EKeyCodes::KeyW))
		{
			pos += fwd * movement;
		}

		if (InputManager::Instance()->KeyHeld(EKeyCodes::KeyA))
		{
			pos -= rig * movement;
		}

		if (InputManager::Instance()->KeyHeld(EKeyCodes::KeyS))
		{
			pos -= fwd * movement;
		}

		if (InputManager::Instance()->KeyHeld(EKeyCodes::KeyD))
		{
			pos += rig * movement;
		}

		if (InputManager::Instance()->KeyHeld(EKeyCodes::KeySpace))
		{
			pos += up * movement;
		}

		if (InputManager::Instance()->KeyHeld(EKeyCodes::ModLCTRL))
		{
			pos -= up * movement;
		}

		m_data.position = pos;

		UpdateCamera();
	}

	void UpdateMouseRotation(glm::vec2 pos)
	{
		if (m_ignore_mouse || m_data.mode != ECameraType::FPS)
		{
			return;
		}

		if (!m_mouse_pos_init)
		{
			m_last_mouse_pos = pos;
			m_mouse_pos_init = true;
		}

		glm::vec2 diff   = pos - m_last_mouse_pos;
		m_last_mouse_pos = pos;

		diff.x *= m_mouse_rotation_speed;
		diff.y *= m_mouse_rotation_speed;

		m_data.orientation.y += diff.x;
		m_data.orientation.x -= diff.y;

		float pitch = m_data.orientation.x;

		// Prevent flipping camera

		if (pitch > 89.9f)
		{
			pitch = 89.9f;
		}

		if (pitch < -89.9f)
		{
			pitch = -89.9f;
		}

		m_data.orientation.x = pitch;

		UpdateCamera();
	}

	[[nodiscard]] CameraInfo Info() const
	{
		return m_data;
	}

	[[nodiscard]] glm::mat4& Projection()
	{
		return m_projection_matrix;
	}

	[[nodiscard]] glm::mat4& View()
	{
		return m_view_matrix;
	}

private:

	void UpdateCamera()
	{
		if (m_data.mode == ECameraType::FPS)
		{
			UpdateFPSCamera();
		}

		const std::string message = "Pos: " + std::to_string(m_data.position.x) + ", " +
				std::to_string(m_data.position.y) + ", " + std::to_string(m_data.position.z) +
				" | Ori: " + std::to_string(m_data.orientation.x) + ", " + std::to_string(m_data.orientation.y) +
				", " + std::to_string(m_data.orientation.z);

		std::printf("\r%s", message.c_str());
	}

	void UpdateFPSCamera()
	{
		const float pitch = m_data.orientation.x;
		const float yaw   = m_data.orientation.y;

		fwd.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		fwd.y = sin(glm::radians(pitch));
		fwd.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		fwd   = glm::normalize(fwd);

		rig = glm::normalize(glm::cross(fwd, glm::vec3(0.0f, 1.0f, 0.0f)));
		up  = glm::normalize(glm::cross(rig, fwd));

		m_view_matrix = glm::lookAt(m_data.position, m_data.position + fwd, up);
	}

	const float m_movement_speed       = 1.0f;
	const float m_rotation_speed       = 1.0f;
	const float m_mouse_rotation_speed = 0.1f;

	glm::mat4 m_projection_matrix;
	glm::mat4 m_view_matrix;

	glm::vec3 fwd;
	glm::vec3 rig;
	glm::vec3 up;

	CameraInfo m_data;

	bool      m_mouse_pos_init = false;
	glm::vec2 m_last_mouse_pos;
	bool      m_ignore_mouse = true;
};
