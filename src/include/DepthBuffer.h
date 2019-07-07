#pragma once
namespace VkRes
{
	class DepthBuffer
	{
	public:

		DepthBuffer() = default;

		DepthBuffer(vk::PhysicalDevice      _physical_device,
		            vk::Device              _device,
		            uint32_t                _width,
		            uint32_t                _height,
		            vk::SampleCountFlagBits _sample_count,
		            vk::ImageTiling         _image_tiling,
		            VkRes::Command          _cmd,
		            vk::Queue               _queue)
		{
			m_format = GetDepthFormat(_physical_device);

			auto image_data = VkRes::CreateImage(_device, _physical_device, _width, _height,
			                                     m_format, 1, _sample_count, _image_tiling,
			                                     vk::ImageUsageFlagBits::eDepthStencilAttachment,
			                                     vk::MemoryPropertyFlagBits::eDeviceLocal);

			m_image        = std::get<0>(image_data);
			m_image_memory = std::get<1>(image_data);
			m_image_view   = VkRes::CreateImageView(_device, m_image, m_format, vk::ImageAspectFlagBits::eDepth, 1);

			VkRes::TransitionImageLayout(_device, _cmd, _queue,
			                             m_image, m_format, vk::ImageLayout::eUndefined,
			                             vk::ImageLayout::eDepthStencilAttachmentOptimal, 1u);

			CreateAttachmentDesc(m_format, _sample_count);
		}

		void Destroy(vk::Device _device)
		{
			if (m_image_view != nullptr)
			{
				_device.destroyImageView(m_image_view);
				m_image_view = nullptr;
			}

			if (m_image != nullptr)
			{
				_device.destroyImage(m_image);
				m_image = nullptr;
			}

			if (m_image_memory != nullptr)
			{
				_device.freeMemory(m_image_memory);
				m_image_memory = nullptr;
			}
		}

		vk::Format Format() const
		{
			return m_format;
		}

		vk::ImageView& ImageView()
		{
			return m_image_view;
		}

		vk::AttachmentDescription& GetAttachmentDesc()
		{
			return m_attachment_desc;
		}

	private:

		vk::Format GetDepthFormat(vk::PhysicalDevice _physical_device) const
		{
			return VkRes::FindSupportedFormat(_physical_device,
			                                  {
				                                  vk::Format::eD32Sfloat,
				                                  vk::Format::eD32SfloatS8Uint
			                                  },
			                                  vk::ImageTiling::eOptimal,
			                                  vk::FormatFeatureFlagBits::eDepthStencilAttachment);
		}

		void CreateAttachmentDesc(vk::Format format, vk::SampleCountFlagBits num_samples)
		{
			m_attachment_desc.format         = format;
			m_attachment_desc.samples        = num_samples;
			m_attachment_desc.loadOp         = vk::AttachmentLoadOp::eClear;
			m_attachment_desc.storeOp        = vk::AttachmentStoreOp::eDontCare;
			m_attachment_desc.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
			m_attachment_desc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			m_attachment_desc.initialLayout  = vk::ImageLayout::eUndefined;
			m_attachment_desc.finalLayout    = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		}

		vk::Image        m_image        = nullptr;
		vk::ImageView    m_image_view   = nullptr;
		vk::DeviceMemory m_image_memory = nullptr;

		vk::AttachmentDescription m_attachment_desc;
		vk::Format                m_format;
	};
}
