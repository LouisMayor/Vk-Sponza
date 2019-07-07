#include "include/Model.h"

extern VkGen::VkGenerator g_VkGenerator;
extern Logger             g_Logger;

Model::Model(vk::Device _device, vk::PhysicalDevice _physical_device, uint32_t _num_of_buffers)
{
	m_descriptor_sets.resize(_num_of_buffers);
}

void Model::LoadMesh(vk::Device         _device,
                     vk::PhysicalDevice _physical_device,
                     std::string        _dir,
                     std::string        _name)
{
	m_model_directory = _dir;
	m_model_name      = _name;

	m_mesh.Load(_device, _physical_device, _dir, _name, m_materials, m_shapes);

	g_Logger.Info<Model>(*this, "Model Loaded");
}

void Model::LoadTexture(VkRes::Command _cmd, std::string _dir, std::string _name)
{
	if (m_render_type == ERenderType::Diffuse)
	{
		Diffuse* val = std::get_if<Diffuse>(&m_textures);
		if (val != nullptr)
		{
			val->DiffuseTexture = VkRes::Texture<VkRes::ETextureLoader::STB>(g_VkGenerator.Device(),
			                                                                 g_VkGenerator.PhysicalDevice(),
			                                                                 _cmd,
			                                                                 g_VkGenerator.GraphicsQueue(),
			                                                                 _dir,
			                                                                 _name);
		}
	}
}

void Model::CreateDescriptorSetLayout(int _dst_binding)
{
	m_descriptor_set_layout_binding =
			VkRes::CreateDescriptorSetLayout(vk::DescriptorType::eCombinedImageSampler,
			                                 vk::ShaderStageFlagBits::eFragment,
			                                 _dst_binding,
			                                 nullptr);
}

void Model::CreateDescriptorSet(const int _buffer_index, vk::DescriptorSet& _set, const vk::Sampler& _sampler)
{
	m_descriptor_sets[_buffer_index] = VkRes::CreateDescriptorSet(_set,
	                                                              vk::DescriptorType::eCombinedImageSampler,
	                                                              nullptr,
	                                                              &GetImageInfo(_sampler),
	                                                              m_descriptor_set_layout_binding.binding);
}

void Model::Destroy(vk::Device _device)
{
	if (m_render_type == ERenderType::Diffuse)
	{
		Diffuse* val = std::get_if<Diffuse>(&m_textures);
		if (val != nullptr)
		{
			val->DiffuseTexture.Destroy(_device);
		}
	}

	m_mesh.Destroy(_device);
}

std::ostream& operator<<(std::ostream& _ostream, const Model& _other)
{
	_ostream << std::endl <<
			"Model name: " << _other.m_model_name << std::endl <<
			"Model directory: " << _other.m_model_directory << std::endl <<
			"Number of vertices " << _other.m_mesh.vertex_count << std::endl <<
			"Number of indices " << _other.m_mesh.index_count << std::endl <<
			"Number of sub-models " << _other.m_shapes.size() << std::endl;

	for (auto& i : _other.m_shapes)
	{
		_ostream << i.name << std::endl;
	}

	return _ostream;
}
