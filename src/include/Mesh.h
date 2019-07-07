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

private:
	std::vector<VertexPosUVNormal>   m_vertices;
	std::vector<uint32_t> m_indices;
	VkRes::Buffer         m_vertex_buffer;
	VkRes::Buffer         m_index_buffer;
};
