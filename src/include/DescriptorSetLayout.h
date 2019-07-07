#pragma once

namespace VkRes
{
	class DescriptorSetLayout
	{
	public:
		DescriptorSetLayout() = default;

		void CreateLayouts(vk::Device _device)
		{
			const vk::DescriptorSetLayoutCreateInfo create_info =
			{
				{},
				m_layout_bindings.size(),
				m_layout_bindings.data()
			};

			m_layouts.resize(3);
			if (m_layouts.size() > 0)
			{
				for(auto &desc : m_layouts)
				{
					const auto result = _device.createDescriptorSetLayout(&create_info, nullptr, &desc);
					assert(("Failed to create a descriptor set layout", result == vk::Result::eSuccess));
				}				
			}
			else
			{
				throw std::logic_error("DescriptorSetLayout has no values");
			}
		}

		void Destroy(vk::Device _device)
		{
			for (auto& desc : m_layouts)
			{
				_device.destroyDescriptorSetLayout(desc);
			}

			m_layouts.clear();
			m_layout_bindings.clear();
		}

		void Add(vk::DescriptorSetLayoutBinding _layout)
		{
			m_layout_bindings.emplace_back(_layout);
		}

		void Remove(vk::DescriptorSetLayoutBinding _layout)
		{
			m_layout_bindings.erase(std::remove(m_layout_bindings.begin(), m_layout_bindings.end(), _layout));
			m_layout_bindings.shrink_to_fit();
		}

		[[nodiscard]] vk::DescriptorSetLayout* Get()
		{
			return m_layouts.data();
		}

		[[nodiscard]] uint32_t BindingCount() const
		{
			return m_layout_bindings.size();
		}

	private:
		std::vector<vk::DescriptorSetLayout>        m_layouts;
		std::vector<vk::DescriptorSetLayoutBinding> m_layout_bindings;
	};
}
