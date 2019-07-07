// macro 'TINYOBJLOADER_IMPLEMENTATION' defined in CubeDemo.cpp

#include "include/Mesh.h"

void Mesh::Destroy(vk::Device _device)
{
	m_vertex_buffer.Destroy(_device);
	m_index_buffer.Destroy(_device);
}

void Mesh::MapData(vk::Device _device, int _shape_count)
{
	for(int index = 0; index < _shape_count; ++index)
	{
		auto vert = m_vertex_buffer;
		auto indi = m_index_buffer;

		vert.Map(_device);
		std::memcpy(vert.Data(), m_vertices.data(), vertex_count * sizeof(VertexPosUVNormal));
		vert.Unmap(_device);

		indi.Map(_device);
		std::memcpy(indi.Data(), m_indices.data(), index_count * sizeof(int));
		indi.Unmap(_device);
	}
}

void Mesh::Load(vk::Device                        _device,
                vk::PhysicalDevice                _physical_device,
                std::string                       _dir,
                std::string                       _name,
                std::vector<tinyobj::material_t>& _materials,
                std::vector<tinyobj::shape_t>&    _shapes)
{
	tinyobj::attrib_t attrib;
	std::string       err;
	std::string       location = (_dir + _name);

	if (!tinyobj::LoadObj(&attrib, &_shapes, &_materials, &err, location.c_str(), ""))
	{
		throw std::runtime_error(err);
	}

	std::unordered_map<VertexPosUVNormal, uint32_t> unique_vertices = {};

	int shape_counter = 0;
	for (const auto& shape : _shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			VertexPosUVNormal vertex = {};

			bool has_pos = index.vertex_index != -1;

			if (has_pos)
			{
				vertex.pos =
				{
					attrib.vertices[3 * index.vertex_index + 2],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 0]
				};
			}
			else
			{
				vertex.pos =
				{
					0.0,
					0.0,
					0.0
				};
			}

			vertex.color = { 1.0f, 1.0f, 1.0f };

			bool has_uv = index.texcoord_index != -1;

			if (has_uv)
			{
				vertex.texCoord =
				{
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}
			else
			{
				vertex.texCoord =
				{
					0.0,
					0.0
				};
			}

			bool has_normals = index.normal_index != -1;

			if (has_normals)
			{
				vertex.normal =
				{
					attrib.normals[3 * index.normal_index + 2],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 0]
				};
			}
			else
			{
				vertex.normal =
				{
					0.0,
					0.0,
					0.0
				};
			}			

			if (unique_vertices.count(vertex) == 0)
			{
				unique_vertices[vertex] = static_cast<uint32_t>(m_vertices.size());
				m_vertices.push_back(vertex);
			}

			m_indices.push_back(unique_vertices[vertex]);
		}
		shape_counter++;
	}

	vertex_count = m_vertices.size();	
	index_count = m_indices.size();

	m_vertex_buffer = VkRes::Buffer(_device, _physical_device, vertex_count * sizeof(VertexPosUVNormal), vk::BufferUsageFlagBits::eVertexBuffer);
	m_index_buffer = VkRes::Buffer(_device, _physical_device, index_count * sizeof(int), vk::BufferUsageFlagBits::eIndexBuffer);

	MapData(_device, 1);
}

void Mesh::Draw(vk::CommandBuffer _cmd_buffer)
{
	vk::DeviceSize offsets[] = { 0 };
	_cmd_buffer.bindVertexBuffers(0, 1, &m_vertex_buffer.BufferData(), offsets);
	_cmd_buffer.bindIndexBuffer(m_index_buffer.BufferData(), 0, vk::IndexType::eUint32);
	const int num_of_indices = static_cast<uint32_t>(m_indices.size());
	_cmd_buffer.drawIndexed(num_of_indices, 1, 0, 0, 0);
}
