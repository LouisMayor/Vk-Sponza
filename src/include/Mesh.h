#pragma once

#include <string>
#include "VulkanObjects.h"
#include <tiny_obj_loader.h>
#include "VertexTypes.h"

class Mesh
{
public:
	Mesh() = default;

	void Destroy(vk::Device);

	void MapData(vk::Device, int);

	void Load(vk::Device                       ,
	          vk::PhysicalDevice               ,
	          std::string                      ,
	          std::string                      ,
	          std::vector<tinyobj::material_t>&,
	          std::vector<tinyobj::shape_t>&   );

	void Draw(vk::CommandBuffer);

	size_t vertex_count;
	size_t index_count;

	[[nodiscard]] glm::mat4x4 Matrix() const
	{
		return m_matrix;
	}

	[[nodiscard]] glm::vec3 Position() const
	{
		return m_position;
	}

	[[nodiscard]] glm::vec3 Rotation() const
	{
		return m_rotation;
	}

	[[nodiscard]] glm::vec3 Scale() const
	{
		return m_scale;
	}

	void Position(const glm::vec3 _position)
	{
		m_position = _position;
		CalculateMatrix();
	}

	void Rotation(const glm::vec3 _rotation)
	{
		m_rotation = _rotation;
		CalculateMatrix();
	}

	void Scale(const glm::vec3 _scale)
	{
		m_scale = _scale;
		CalculateMatrix();
	}

private:

	void CalculateMatrix()
	{
		// calculating using axis angle
		const auto rotationX = glm::rotate(glm::mat4x4(1.0f), m_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		const auto rotationY = glm::rotate(glm::mat4x4(1.0f), m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		const auto rotationZ = glm::rotate(glm::mat4x4(1.0f), m_rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		// combine all rotations
		const auto rotation    = rotationZ * rotationX * rotationY;
		const auto translation = glm::translate(glm::mat4x4(1.0f), m_position);
		const auto scale       = glm::scale(glm::mat4x4(1.0f), m_scale);

		m_matrix = translation * rotation * scale;
	}

	std::vector<VertexPosUVNormal> m_vertices;
	std::vector<uint32_t>          m_indices;
	VkRes::Buffer                  m_vertex_buffer;
	VkRes::Buffer                  m_index_buffer;

	glm::mat4x4 m_matrix   = glm::mat4x4(1.0f);
	glm::vec3   m_position = glm::vec3(0.0f);
	glm::vec3   m_rotation = glm::vec3(0.0f);
	glm::vec3   m_scale    = glm::vec3(1.0f);
};
