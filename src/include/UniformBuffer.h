#pragma once

namespace VkRes
{
	template <typename Data, EDataUsageFlags UsageFlags> class UniformBuffer
	{
	public:

		UniformBuffer() = default;

		UniformBuffer(vk::Device _device, vk::PhysicalDevice _physical_device, uint32_t num_of_buffers = 1,
		              bool       dynamic                                                               = false) : m_dynamic(
			dynamic)
		{
			assert(("invalid number of buffers"), num_of_buffers > 0);

			m_uniform_type = m_dynamic ?
				                 vk::DescriptorType::eUniformBufferDynamic :
				                 vk::DescriptorType::eUniformBuffer;

			m_buffer_size = sizeof(Data);
			m_buffers.resize(num_of_buffers);
			m_descriptor_sets.resize(num_of_buffers);

			for (auto& buffer : m_buffers)
			{
				std::get<0>(buffer) = VkRes::Buffer(_device, _physical_device, m_buffer_size,
				                                    vk::BufferUsageFlagBits::eUniformBuffer);
			}
		}

		void Destroy(vk::Device _device)
		{
			for (auto& i : m_buffers)
			{
				std::get<0>(i).Destroy(_device);
			}
		}

		void CreateDescriptorSetLayout(const vk::ShaderStageFlags _stage_flags,
		                               const int                  _dst_binding)
		{
			m_descriptor_set_layout_binding =
					VkRes::CreateDescriptorSetLayout(m_uniform_type,
					                                 _stage_flags,
					                                 _dst_binding,
					                                 nullptr);
		}

		void CreateDescriptorSet(const int          _buffer_index,
		                         vk::DescriptorSet& _set)
		{
			m_descriptor_sets[_buffer_index] = VkRes::CreateDescriptorSet(_set,
			                                                              m_uniform_type,
			                                                              &DescInfo(_buffer_index),
			                                                              nullptr,
			                                                              m_descriptor_set_layout_binding.binding);
		}

		void UpdateDescriptorSet(vk::Device _device, const uint32_t _index)
		{
			_device.updateDescriptorSets(1, &DescSet(_index), 0, nullptr);
		}

		void Map(vk::Device _device, const int _buffer_index = 0)
		{
			VkRes::Buffer buffer = std::get<0>(m_buffers[_buffer_index]);

			buffer.Map(_device);
			std::memcpy(buffer.Data(), &GetData(_buffer_index), static_cast<size_t>(TotalBufferSize()));
			buffer.Unmap(_device);
		}

		[[nodiscard]] bool WantsPerFrameUpdate() const
		{
			return static_cast<bool>(UsageFlags & PerFrame);
		}

		[[nodiscard]] bool WantsOnResizeUpdate() const
		{
			return static_cast<bool>(UsageFlags & OnResize);
		}

		[[nodiscard]] uint32_t BufferCount() const
		{
			return m_number_of_buffers;
		}

		[[nodiscard]] vk::Buffer& GetBuffer(const int _buffer_index = 0)
		{
			return std::get<0>(m_buffers[_buffer_index]).BufferData();
		}

		[[nodiscard]] Data& GetData(const int _buffer_index = 0)
		{
			return std::get<1>(m_buffers[_buffer_index]);
		}

		[[nodiscard]] vk::DescriptorSetLayoutBinding& DescLayoutBinding()
		{
			return m_descriptor_set_layout_binding;
		}

		[[nodiscard]] vk::WriteDescriptorSet& DescSet(const int _buffer_index = 0)
		{
			return m_descriptor_sets[_buffer_index];
		}

		[[nodiscard]] vk::DescriptorBufferInfo& DescInfo(const int _buffer_index = 0)
		{
			m_info.setBuffer(GetBuffer(_buffer_index));
			m_info.setOffset(0);
			m_info.setRange(m_buffer_size);

			return m_info;
		}

	private:

		vk::DeviceSize TotalBufferSize()
		{
			return m_buffer_size * m_buffers.size();
		}

		bool                                         m_dynamic = false;
		vk::DescriptorType                           m_uniform_type;
		uint32_t                                     m_number_of_buffers = 0;
		vk::DeviceSize                               m_buffer_size;
		std::vector<std::tuple<VkRes::Buffer, Data>> m_buffers;
		vk::DescriptorBufferInfo                     m_info;
		vk::DescriptorSetLayoutBinding               m_descriptor_set_layout_binding;
		std::vector<vk::WriteDescriptorSet>          m_descriptor_sets;
	};
}
