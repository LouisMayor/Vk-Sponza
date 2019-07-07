#pragma once

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

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
		glm::vec3   position;
		glm::vec3   orientation;
		ECameraType mode;
	};

	Camera() = default;

	template <ECameraType CameraType = ECameraType::Static> Camera operator=(const CameraInfo _creation_data) const
	{
		Camera tmp;
		tmp.m_data      = _creation_data;
		tmp.m_data.mode = CameraType;

		tmp.UpdateCamera();
		return tmp;
	}

	void Update(float _delta)
	{
		if (m_data.mode == ECameraType::Static)
		{
			return;
		}

		glm::vec3 pos = m_data.position;
		glm::vec3 fwd = m_view_matrix[2];
		glm::vec3 up  = m_view_matrix[1];
		glm::vec3 rig = m_view_matrix[0];

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
		if (m_data.mode != ECameraType::FPS)
		{
			return;
		}

		pos.x *= m_mouse_rotation_speed;
		pos.y *= m_mouse_rotation_speed;

		m_data.orientation.y += pos.x;
		m_data.orientation.y -= pos.y;

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
	}

	void UpdateFPSCamera()
	{
		glm::vec3 right   = m_view_matrix[0];
		glm::vec3 up      = m_view_matrix[1];
		glm::vec3 forward = m_view_matrix[2];

		const float pitch = m_data.orientation.x;
		const float yaw   = m_data.orientation.y;

		forward.x = std::cosf(glm::radians(yaw) * std::cos(glm::radians(pitch)));
		forward.y = std::sinf(glm::radians(pitch));
		forward.z = std::sinf(glm::radians(yaw) * std::cos(glm::radians(pitch)));
		forward   = glm::normalize(forward);

		right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
		up    = glm::normalize(glm::cross(right, forward));

		m_view_matrix = glm::lookAt(m_data.position, m_data.position + forward, up);
	}

	const float m_movement_speed       = 1.0f;
	const float m_rotation_speed       = 1.0f;
	const float m_mouse_rotation_speed = 1.0f;

	glm::mat4 m_projection_matrix;
	glm::mat4 m_view_matrix;

	CameraInfo m_data;

	bool m_mouse_pos_init = false;
	glm::vec2 m_last_mouse_pos;
};
