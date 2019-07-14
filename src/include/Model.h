#pragma once

#include "Mesh.h"

#include <string>
#include <variant>

enum class ERenderType
{
	Diffuse,
	DiffuseSpec,
	DiffuseSpecNormal,
	ShaderBased
};

struct ShaderBased
{
	/* Nothing set by the program, all colours set via shader */
};

struct Diffuse
{
	VkRes::Texture<VkRes::ETextureLoader::STB> DiffuseTexture;
};

struct DiffuseSpec
{
	VkRes::Texture<VkRes::ETextureLoader::STB> DiffuseTexture;
	VkRes::Texture<VkRes::ETextureLoader::STB> SpecularTexture;
};

struct DiffuseSpecNormal
{
	VkRes::Texture<VkRes::ETextureLoader::STB> DiffuseTexture;
	VkRes::Texture<VkRes::ETextureLoader::STB> SpecularTexture;
	VkRes::Texture<VkRes::ETextureLoader::STB> NormalTexture;
};

class Model
{
public:

	Model() = default;

	Model(vk::Device, vk::PhysicalDevice, uint32_t);

	void Destroy(vk::Device);

	template <ERenderType render_type> void SetTextureSupport()
	{
		if constexpr (render_type == ERenderType::Diffuse)
		{
			m_textures = Diffuse();
		}
		else if constexpr (render_type == ERenderType::DiffuseSpec)
		{
			m_textures = DiffuseSpec();
		}
		else if constexpr (render_type == ERenderType::DiffuseSpecNormal)
		{
			m_textures = DiffuseSpecNormal();
		}

		m_render_type = render_type;
	}

	void LoadMesh(vk::Device, vk::PhysicalDevice, std::string, std::string);

	void LoadTexture(VkRes::Command, std::string, std::string);

	void CreateDescriptorSetLayout(int);

	void CreateDescriptorSet(int, vk::DescriptorSet&, const vk::Sampler&);

	void UpdateDescriptorSet(vk::Device _device, const uint32_t _index)
	{
		_device.updateDescriptorSets(1, &DescSet(_index), 0, nullptr);
	}

	[[nodiscard]] vk::ImageView& GetImageView()
	{
		if (m_render_type == ERenderType::Diffuse)
		{
			Diffuse* val = std::get_if<Diffuse>(&m_textures);
			return val->DiffuseTexture.View();
		}

		vk::ImageView tmp;
		return tmp;
	}

	[[nodiscard]] vk::DescriptorSetLayoutBinding& DescLayoutBinding()
	{
		return m_descriptor_set_layout_binding;
	}

	[[nodiscard]] vk::DescriptorImageInfo& GetImageInfo(const vk::Sampler& _sampler)
	{
		m_image_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		m_image_info.imageView   = GetImageView();
		m_image_info.sampler     = _sampler;

		return m_image_info;
	}

	[[nodiscard]] vk::WriteDescriptorSet& DescSet(const int _buffer_index = 0)
	{
		return m_descriptor_sets[_buffer_index];
	}

	friend std::ostream& operator<<(std::ostream& _ostream, const Model& _other);

	[[nodiscard]] Mesh& MeshInstance()
	{
		return m_mesh;
	}

	[[nodiscard]] glm::mat4x4 Matrix() const
	{
		return m_mesh.Matrix();
	}

	[[nodiscard]] glm::vec3 Position() const
	{
		return m_mesh.Position();
	}

	[[nodiscard]] glm::vec3 Rotation() const
	{
		return m_mesh.Rotation();
	}

	[[nodiscard]] glm::vec3 Scale() const
	{
		return m_mesh.Scale();
	}

	void Position(const glm::vec3 _position)
	{
		m_mesh.Position(_position);
	}

	void Rotation(const glm::vec3 _rotation)
	{
		m_mesh.Position(_rotation);
	}

	void Scale(const glm::vec3 _scale)
	{
		m_mesh.Position(_scale);
	}

private:

	std::string m_model_directory;
	std::string m_model_name;

	Mesh                                                  m_mesh;
	std::variant<Diffuse, DiffuseSpec, DiffuseSpecNormal> m_textures;
	ERenderType                                           m_render_type;

	std::vector<tinyobj::shape_t>    m_shapes;
	std::vector<tinyobj::material_t> m_materials;

	vk::DescriptorImageInfo             m_image_info;
	vk::DescriptorBufferInfo            m_info;
	vk::DescriptorSetLayoutBinding      m_descriptor_set_layout_binding;
	std::vector<vk::WriteDescriptorSet> m_descriptor_sets;
};
