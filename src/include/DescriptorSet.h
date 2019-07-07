#pragma once

namespace VkRes
{
	class DescriptorSet
	{
	public:
		DescriptorSet() = default;

		DescriptorSet(vk::Device _device, const uint32_t _num_of_sets, vk::DescriptorPool _pool, vk::DescriptorSetLayout* _layouts)
		{
			const vk::DescriptorSetAllocateInfo alloc_info =
			{
				_pool,
				_num_of_sets,
				_layouts
			};

			m_sets.resize(_num_of_sets);

			const auto result = _device.allocateDescriptorSets(&alloc_info, m_sets.data());
			assert(("Failed to create a descriptor pool", result == vk::Result::eSuccess));
		}

		[[nodiscard]] vk::DescriptorSet& Get(uint32_t _index)
		{
			if (_index > m_sets.size())
			{
				const std::string message = std::to_string(_index) + " > " + std::to_string(m_sets.size());
				throw std::out_of_range(message);
			}

			return m_sets[_index];
		}

	private:
		std::vector<vk::DescriptorSet> m_sets;
	};
}
