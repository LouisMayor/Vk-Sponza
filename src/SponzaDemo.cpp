#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "include/SponzaDemo.h"
#include <chrono>

VkSponzaDemo* VkSponzaDemo::m_instance = nullptr;

extern VkGen::VkGenerator g_VkGenerator;
extern Logger             g_Logger;

VkSponzaDemo::VkSponzaDemo()
{
	VkSponzaDemo::m_instance = this;
}

void VkSponzaDemo::Setup()
{
	CreateSwapchain();
	CreateCmdPool();
	CreateCmdBuffers();
	LoadAssets();
	CreateColourResources();
	CreateDepthResources();
	CreateRenderPasses();
	CreateFrameBuffers();
	CreateShaders();
	CreateResources();
	CreateDescriptorLayouts();
	CreateDescriptorPools();
	CreateDescriptorSets();
	CreatePipelines();
	CreateSyncObjects();

	// Update all buffers that rely on swapchain resizing
	for (size_t image = 0; image < m_swapchain.ImageViews().size(); ++image)
	{
		UpdateBufferData(image, true);
	}

	const bool msaa = SponzaDemoSettings::Instance()->use_msaa;

	m_ui_instance.Init(m_swapchain.Extent().width, m_swapchain.Extent().height, g_VkGenerator.WindowHdle());
	m_ui_instance.LoadResources(g_VkGenerator.Device(), g_VkGenerator.PhysicalDevice(), m_shader_directory,
	                            m_pipeline_cache_directory, m_command,
	                            m_render_pass.Pass(), g_VkGenerator.GraphicsQueue(), msaa ?
		                                                                                 SponzaDemoSettings::Instance()->
		                                                                                 GetSampleCount() :
		                                                                                 vk::SampleCountFlagBits::e1);

	m_app_instance.SetWindowTitle("Vulkan Sponza Demo");
	m_app_instance.Start();
}

void VkSponzaDemo::Run()
{
	float init_time      = 0.0f;
	bool  stop_execution = false;

	while (!stop_execution)
	{
		m_total_time  = static_cast<float>(glfwGetTime());
		m_frame_delta = m_total_time - init_time;
		init_time     = m_total_time;

		m_app_instance.Update(m_frame_delta);
		stop_execution     = m_app_instance.ShouldStop();
		m_settings_updated = SponzaDemoSettings::Instance()->Updated(true);

		if (!(m_settings_updated & Settings::SettingUpdateFlags::None))
		{
			if (m_settings_updated & Settings::SettingUpdateFlags::DescriptorsRecreation)
			{
				RecreateDescriptors();
			}

			if (m_settings_updated & Settings::SettingUpdateFlags::SwapchainRecreation)
			{
				RecreateSwapchain();
			}

			m_settings_updated = Settings::SettingUpdateFlags::None;
		}

		RecordCmdBuffer();
		SubmitQueue();
	}
}

void VkSponzaDemo::Shutdown()
{
	g_VkGenerator.Device().waitIdle();

	m_desc_set_layouts.Destroy(g_VkGenerator.Device());
	m_desc_pool.Destroy(g_VkGenerator.Device());

	m_view_ubo.Destroy(g_VkGenerator.Device());
	m_cube_ubo.Destroy(g_VkGenerator.Device());
	m_dir_light_ubo.Destroy(g_VkGenerator.Device());
	m_sampler.Destroy(g_VkGenerator.Device());

	for (auto& i : m_render_list)
	{
		i.Destroy(g_VkGenerator.Device());
	}

	m_ui_instance.Destroy(g_VkGenerator.Device());

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_inflight_fences[i].Destroy(g_VkGenerator.Device());
		m_image_available_semaphores[i].Destroy(g_VkGenerator.Device());
		m_render_finished_semaphores[i].Destroy(g_VkGenerator.Device());
	}

	m_non_lit_tex_vert.Destroy(g_VkGenerator.Device());
	m_non_lit_tex_frag.Destroy(g_VkGenerator.Device());
	m_non_lit_graphics_pipeline.Destroy(g_VkGenerator.Device());

	for (auto& i : m_framebuffers)
	{
		i.Destroy(g_VkGenerator.Device());
	}

	m_render_pass.Destroy(g_VkGenerator.Device());
	m_depth_buffer.Destroy(g_VkGenerator.Device());
	m_backbuffer.Destroy(g_VkGenerator.Device());
	m_command.Destroy(g_VkGenerator.Device());
	m_swapchain.Destroy(g_VkGenerator.Device());

	m_app_instance.Close();
}

VkSponzaDemo* VkSponzaDemo::Instance()
{
	return m_instance;
}

void VkSponzaDemo::RecreateDescriptors()
{
	g_VkGenerator.Device().waitIdle();

	m_desc_set_layouts.Destroy(g_VkGenerator.Device());
	m_desc_pool.Destroy(g_VkGenerator.Device());

	CreateDescriptorLayouts();
	CreateDescriptorPools();
	CreateDescriptorSets();
}

VkBool32 VkSponzaDemo::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      _message_severity,
                                     VkDebugUtilsMessageTypeFlagsEXT             _message_type,
                                     const VkDebugUtilsMessengerCallbackDataEXT* _p_callback_data,
                                     void*                                       _p_user_data)
{
	std::string message = "validation layer: ";
	if (_message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		g_Logger.Error(message + _p_callback_data->pMessage);
	}
	else if (_message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		g_Logger.Warning(message + _p_callback_data->pMessage);
	}
	else
	{
		g_Logger.Info(message + _p_callback_data->pMessage);
	}
	return VK_FALSE;
}

void VkSponzaDemo::LoadAssets()
{
	Model cube(g_VkGenerator.Device(), g_VkGenerator.PhysicalDevice(), m_swapchain.ImageViews().size());
	cube.SetTextureSupport<ERenderType::Diffuse>();
	cube.LoadMesh(g_VkGenerator.Device(), g_VkGenerator.PhysicalDevice(), m_model_directory, "cube_normals.obj");
	cube.LoadTexture(m_command, m_texture_directory, "texture.jpg");

	m_render_list.emplace_back(cube);
}

void VkSponzaDemo::SubmitQueue()
{
	const auto device                    = g_VkGenerator.Device();
	const auto fence                     = &m_inflight_fences[m_current_frame].FenceInstance();
	const auto image_available_semaphore = m_image_available_semaphores[m_current_frame].SemaphoreInstance();
	const auto graphics_queue            = g_VkGenerator.GraphicsQueue();
	const auto present_queue             = g_VkGenerator.PresentQueue();

	device.waitForFences(1, fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

	const auto result_val = device.acquireNextImageKHR(m_swapchain.SwapchainInstance(),
	                                                   std::numeric_limits<uint64_t>::max(),
	                                                   image_available_semaphore,
	                                                   nullptr);
	const uint32_t image_index = result_val.value;

	if (result_val.result == vk::Result::eErrorOutOfDateKHR)
	{
		RecreateSwapchain();
	}
	else if (result_val.result != vk::Result::eSuccess && result_val.result == vk::Result::eSuboptimalKHR)
	{
		g_Logger.Error("Failed to acquire swapchain image");
		return;
	}

	UpdateBufferData(image_index, false);

	const auto command_buffer = m_command.CommandBuffer(image_index);

	vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

	const vk::SubmitInfo submit_info =
	{
		1,
		&image_available_semaphore,
		waitStages,
		1,
		&command_buffer,
		1,
		&m_render_finished_semaphores[m_current_frame].SemaphoreInstance(),
	};

	const auto fence_reset_result = device.resetFences(1, fence);
	assert(("Failed to reset fence", fence_reset_result == vk::Result::eSuccess));

	const auto submit_result = graphics_queue.submit(1, &submit_info, *fence);
	assert(("Failed to submit a draw queue", submit_result == vk::Result::eSuccess));

	const vk::PresentInfoKHR present_info =
	{
		1,
		&m_render_finished_semaphores[m_current_frame].SemaphoreInstance(),
		1,
		&m_swapchain.SwapchainInstance(),
		&image_index
	};

	const auto present_result = present_queue.presentKHR(&present_info);

	if (present_result == vk::Result::eErrorOutOfDateKHR || present_result == vk::Result::eSuboptimalKHR || m_buffer_resized)
	{
		RecreateSwapchain();
		m_buffer_resized = false;
	}
	else if (present_result != vk::Result::eSuccess)
	{
		g_Logger.Error("Failed to present backbuffer");
		return;
	}

	m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VkSponzaDemo::CreateSyncObjects()
{
	m_inflight_fences.resize(MAX_FRAMES_IN_FLIGHT);
	m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_inflight_fences[i]            = VkRes::Fence(g_VkGenerator.Device(), vk::FenceCreateFlagBits::eSignaled);
		m_image_available_semaphores[i] = VkRes::Semaphore(g_VkGenerator.Device(), {});
		m_render_finished_semaphores[i] = VkRes::Semaphore(g_VkGenerator.Device(), {});
	}
}

void VkSponzaDemo::RecordCmdBuffer()
{
	g_VkGenerator.Device().waitIdle();

	vk::CommandBufferBeginInfo begin_info =
	{
		vk::CommandBufferUsageFlagBits::eSimultaneousUse,
		nullptr
	};

	std::array<vk::ClearValue, 3> clear_values = {};
	clear_values[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
	clear_values[1].depthStencil.setDepth(1.0f);
	clear_values[1].depthStencil.setStencil(0);
	clear_values[2].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});

	m_ui_instance.PrepNextFrame(m_frame_delta, m_total_time);
	m_ui_instance.Update(g_VkGenerator.Device(), g_VkGenerator.PhysicalDevice());

	for (auto buffer_index = 0; buffer_index < m_command.CommandBufferCount(); ++buffer_index)
	{
		m_command.BeginRecording(&begin_info, buffer_index);

		vk::RenderPassBeginInfo render_pass_begin_info =
		{
			m_render_pass.Pass(),
			m_framebuffers[buffer_index].Buffer(),
			vk::Rect2D{vk::Offset2D{0, 0}, m_swapchain.Extent()},
			3,
			clear_values.data()
		};

		m_command.BeginRenderPass(&render_pass_begin_info, vk::SubpassContents::eInline, buffer_index);

		m_command.SetViewport(0, static_cast<float>(m_swapchain.Extent().width), static_cast<float>(m_swapchain.Extent().height),
		                      0.0f, 1.0f, buffer_index);

		m_command.SetScissor(0, static_cast<float>(m_swapchain.Extent().width), static_cast<float>(m_swapchain.Extent().height),
		                     buffer_index);

		m_command.BindPipeline(vk::PipelineBindPoint::eGraphics, m_active_pipeline->Pipeline(), buffer_index);

		m_command.PushConstants<float>(m_total_time, m_active_pipeline->PipelineLayout(),
		                               vk::ShaderStageFlagBits::eFragment,
		                               buffer_index);

		m_command.BindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_active_pipeline->PipelineLayout(),
		                             &m_desc_sets.Get(buffer_index), buffer_index);

		for (auto& model : m_render_list)
		{
			model.MeshInstance().Draw(m_command.CommandBuffer(buffer_index));
		}

		m_ui_instance.Draw(m_command, buffer_index);

		m_command.EndRenderPass(buffer_index);

		m_command.EndRecording(buffer_index);
	}
}

void VkSponzaDemo::CreateSwapchain()
{
	m_swapchain = VkRes::Swapchain(g_VkGenerator.PhysicalDevice(), g_VkGenerator.Device(), g_VkGenerator.Surface(),
	                               g_VkGenerator.SwapchainDetails(), g_VkGenerator.QueueFamily());
}

void VkSponzaDemo::CreateCmdPool()
{
	m_command = VkRes::Command(g_VkGenerator.Device(), g_VkGenerator.QueueFamily());
}

void VkSponzaDemo::CreateCmdBuffers()
{
	m_command.CreateCmdBuffers(g_VkGenerator.Device(), m_swapchain.ImageViews().size());
}

void VkSponzaDemo::CreateRenderPasses()
{
	vk::AttachmentReference colour_attachment =
	{
		0,
		vk::ImageLayout::eColorAttachmentOptimal
	};

	vk::AttachmentReference depth_attachment =
	{
		1,
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	};

	vk::AttachmentReference colour_resolve_attachment =
	{
		2,
		vk::ImageLayout::eColorAttachmentOptimal
	};

	std::vector<vk::AttachmentDescription> attachments =
	{
		m_backbuffer.GetAttachmentDesc(),
		m_depth_buffer.GetAttachmentDesc()
	};

	if (SponzaDemoSettings::Instance()->use_msaa)
	{
		attachments.emplace_back(m_backbuffer.GetResolveAttachmentDesc());
	}

	m_render_pass = VkRes::RenderPass(attachments,
	                                  &colour_attachment, 1,
	                                  &depth_attachment,
	                                  SponzaDemoSettings::Instance()->use_msaa ?
		                                  &colour_resolve_attachment :
		                                  nullptr, 1,
	                                  vk::PipelineBindPoint::eGraphics, g_VkGenerator.Device());
}

void VkSponzaDemo::CreateFrameBuffers()
{
	const auto image_views = m_swapchain.ImageViews();
	m_framebuffers.resize(image_views.size());

	for (uint32_t i = 0; i < image_views.size(); ++i)
	{
		std::vector<vk::ImageView> attachments =
		{
			image_views[i],
			m_depth_buffer.ImageView()
		};

		if (SponzaDemoSettings::Instance()->use_msaa && SponzaDemoSettings::Instance()->GetSampleCount() > vk::SampleCountFlagBits
			::e1)
		{
			attachments.push_back(m_backbuffer.GetImageView());

			// odd bug: after adding resolve attachement, the framebuffer expects the order to be in reversed order.
			// not sure what this is about, last time going through the vulkan tutorial (C API), didn't run into this.
			std::reverse(attachments.begin(), attachments.end());
		}

		m_framebuffers[i] = VkRes::FrameBuffer(g_VkGenerator.Device(), attachments,
		                                       m_render_pass.Pass(), m_swapchain.Extent(),
		                                       1);
	}
}

void VkSponzaDemo::CreateShaders()
{
	if (m_shader_directory.empty())
	{
		g_Logger.Error("No Shader Directory has been set");
		return;
	}

	m_non_lit_tex_vert = VkRes::Shader(g_VkGenerator.Device(),
	                                   vk::ShaderStageFlagBits::eVertex,
	                                   m_shader_directory,
	                                   m_non_lit_texture_vert + ".spv");

	m_non_lit_tex_frag = VkRes::Shader(g_VkGenerator.Device(),
	                                   vk::ShaderStageFlagBits::eFragment,
	                                   m_shader_directory,
	                                   m_non_lit_texture_frag + ".spv");

	active_vert = &m_non_lit_tex_vert;
	active_frag = &m_non_lit_tex_frag;
}

void VkSponzaDemo::CreatePipelines()
{
	if (!std::filesystem::is_directory(m_pipeline_cache_directory))
	{
		std::filesystem::create_directory(m_pipeline_cache_directory);
	}

	const std::vector<vk::PipelineShaderStageCreateInfo> non_lit_stages
	{
		active_vert->Set(),
		active_frag->Set()
	};

	const bool                    msaa    = SponzaDemoSettings::Instance()->use_msaa;
	const vk::SampleCountFlagBits samples = msaa ?
		                                        SponzaDemoSettings::Instance()->GetSampleCount() :
		                                        vk::SampleCountFlagBits::e1;

	const auto tex_non_lit_binding = VertexPosUV::getBindingDescription();
	const auto tex_non_lit_attrib  = VertexPosUV::getAttributeDescriptions();

	m_non_lit_graphics_pipeline.SetInputAssembler(&tex_non_lit_binding, tex_non_lit_attrib, vk::PrimitiveTopology::eTriangleList,
	                                              VK_FALSE);
	m_non_lit_graphics_pipeline.SetViewport(m_swapchain.Extent(), 0.0f, 1.0f);
	m_non_lit_graphics_pipeline.SetRasterizer(VK_TRUE, VK_TRUE, vk::CompareOp::eLess, samples, VK_FALSE, VK_FALSE);
	m_non_lit_graphics_pipeline.SetShaders(non_lit_stages);
	m_non_lit_graphics_pipeline.SetPushConstants<float>(0, vk::ShaderStageFlagBits::eFragment);
	m_non_lit_graphics_pipeline.CreatePipelineLayout(g_VkGenerator.Device(), m_desc_set_layouts.Get(), 1, 1);
	m_non_lit_graphics_pipeline.CreatePipeline(g_VkGenerator.Device(), m_render_pass.Pass(), m_pipeline_cache_directory,
	                                           "POS_UV");

	m_active_pipeline = &m_non_lit_graphics_pipeline;
}

void VkSponzaDemo::RecreateActivePipeline()
{
	const bool                    msaa    = SponzaDemoSettings::Instance()->use_msaa;
	const vk::SampleCountFlagBits samples = msaa ?
		                                        SponzaDemoSettings::Instance()->GetSampleCount() :
		                                        vk::SampleCountFlagBits::e1;

	m_non_lit_graphics_pipeline.Destroy(g_VkGenerator.Device());

	const std::vector<vk::PipelineShaderStageCreateInfo> non_lit_stages
	{
		active_vert->Set(),
		active_frag->Set()
	};

	const auto tex_non_lit_binding = VertexPosUV::getBindingDescription();
	const auto tex_non_lit_attrib  = VertexPosUV::getAttributeDescriptions();

	m_non_lit_graphics_pipeline.SetInputAssembler(&tex_non_lit_binding, tex_non_lit_attrib,
	                                              vk::PrimitiveTopology::eTriangleList, VK_FALSE);
	m_non_lit_graphics_pipeline.SetViewport(m_swapchain.Extent(), 0.0f, 1.0f);
	m_non_lit_graphics_pipeline.SetRasterizer(VK_TRUE, VK_TRUE, vk::CompareOp::eLess, samples, VK_FALSE, VK_FALSE);
	m_non_lit_graphics_pipeline.SetShaders(non_lit_stages);
	m_non_lit_graphics_pipeline.SetPushConstants<float>(0, vk::ShaderStageFlagBits::eFragment);
	m_non_lit_graphics_pipeline.CreatePipelineLayout(g_VkGenerator.Device(), m_desc_set_layouts.Get(), 1, 1);
	m_non_lit_graphics_pipeline.CreatePipeline(g_VkGenerator.Device(), m_render_pass.Pass(), m_pipeline_cache_directory,
	                                           "POS_UV");

	m_active_pipeline = &m_non_lit_graphics_pipeline;
}

void VkSponzaDemo::CreateColourResources()
{
	const bool                    msaa    = SponzaDemoSettings::Instance()->use_msaa;
	const vk::SampleCountFlagBits samples = msaa ?
		                                        SponzaDemoSettings::Instance()->GetSampleCount() :
		                                        vk::SampleCountFlagBits::e1;

	m_backbuffer = VkRes::RenderTarget(g_VkGenerator.PhysicalDevice(), g_VkGenerator.Device(),
	                                   m_swapchain.Extent().width, m_swapchain.Extent().height, m_swapchain.Format(),
	                                   samples, vk::ImageTiling::eOptimal,
	                                   vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
	                                   vk::MemoryPropertyFlagBits::eDeviceLocal,
	                                   (msaa) ?
		                                   vk::ImageLayout::eColorAttachmentOptimal :
		                                   vk::ImageLayout::ePresentSrcKHR,
	                                   m_command, g_VkGenerator.GraphicsQueue());
}

void VkSponzaDemo::CreateDepthResources()
{
	const bool                    msaa    = SponzaDemoSettings::Instance()->use_msaa;
	const vk::SampleCountFlagBits samples = msaa ?
		                                        SponzaDemoSettings::Instance()->GetSampleCount() :
		                                        vk::SampleCountFlagBits::e1;

	m_depth_buffer = VkRes::DepthBuffer(g_VkGenerator.PhysicalDevice(),
	                                    g_VkGenerator.Device(),
	                                    m_swapchain.Extent().width,
	                                    m_swapchain.Extent().height,
	                                    samples,
	                                    vk::ImageTiling::eOptimal,
	                                    m_command,
	                                    g_VkGenerator.GraphicsQueue());
}

void VkSponzaDemo::CleanSwapchain()
{
	const auto device = g_VkGenerator.Device();

	device.waitIdle();

	m_backbuffer.Destroy(device);
	m_depth_buffer.Destroy(device);
	m_command.FreeCommandBuffers(device);
	m_render_pass.Destroy(device);
	for (auto& i : m_framebuffers)
	{
		i.Destroy(g_VkGenerator.Device());
	}
	m_swapchain.Destroy(device);
}

void VkSponzaDemo::RecreateSwapchain()
{
	int width = 0, height = 0;
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(g_VkGenerator.WindowHdle(), &width, &height);
		glfwWaitEvents();
	}

	g_VkGenerator.Device().waitIdle();
	g_VkGenerator.RefreshSwapchainDetails();

	CleanSwapchain();
	CreateSwapchain();
	CreateCmdBuffers();
	CreateColourResources();
	CreateDepthResources();
	CreateRenderPasses();
	CreateFrameBuffers();
	RecreateActivePipeline();

	for (size_t image = 0; image < m_swapchain.ImageViews().size(); ++image)
	{
		UpdateBufferData(image, true);
	}

	const bool msaa = SponzaDemoSettings::Instance()->use_msaa;

	m_ui_instance.Destroy(g_VkGenerator.Device());
	m_ui_instance.Recreate(g_VkGenerator.Device(), m_swapchain.Extent().width, m_swapchain.Extent().height,
	                       g_VkGenerator.WindowHdle());
	m_ui_instance.LoadResources(g_VkGenerator.Device(), g_VkGenerator.PhysicalDevice(), m_shader_directory,
	                            m_pipeline_cache_directory, m_command,
	                            m_render_pass.Pass(), g_VkGenerator.GraphicsQueue(), msaa ?
		                                                                                 SponzaDemoSettings::Instance()->
		                                                                                 GetSampleCount() :
		                                                                                 vk::SampleCountFlagBits::e1);
}

void VkSponzaDemo::CreateDescriptorLayouts()
{
	m_cube_ubo.CreateDescriptorSetLayout(vk::ShaderStageFlagBits::eVertex, 0);
	m_view_ubo.CreateDescriptorSetLayout(vk::ShaderStageFlagBits::eFragment, 1);

	m_desc_set_layouts.Add(m_cube_ubo.DescLayoutBinding());
	m_desc_set_layouts.Add(m_view_ubo.DescLayoutBinding());

	for (auto& model : m_render_list)
	{
		model.CreateDescriptorSetLayout(2);
		m_desc_set_layouts.Add(model.DescLayoutBinding());
	}

	m_dir_light_ubo.CreateDescriptorSetLayout(vk::ShaderStageFlagBits::eFragment, 3);
	m_desc_set_layouts.Add(m_dir_light_ubo.DescLayoutBinding());

	m_desc_set_layouts.CreateLayouts(g_VkGenerator.Device());
}

void VkSponzaDemo::CreateDescriptorPools()
{
	m_desc_pool.Add(m_cube_ubo.DescLayoutBinding().descriptorType, m_cube_ubo.DescLayoutBinding().binding);
	m_desc_pool.Add(m_view_ubo.DescLayoutBinding().descriptorType, m_view_ubo.DescLayoutBinding().binding);

	for (auto& model : m_render_list)
	{
		m_desc_pool.Add(model.DescLayoutBinding().descriptorType, model.DescLayoutBinding().binding);
	}

	m_desc_pool.Add(m_dir_light_ubo.DescLayoutBinding().descriptorType, m_dir_light_ubo.DescLayoutBinding().binding);

	m_desc_pool.CreatePool(g_VkGenerator.Device(), m_swapchain.ImageViews().size());
}

void VkSponzaDemo::CreateDescriptorSets()
{
	m_desc_sets = VkRes::DescriptorSet(g_VkGenerator.Device(),
	                                   m_swapchain.ImageViews().size(),
	                                   m_desc_pool.Get(),
	                                   m_desc_set_layouts.Get());

	for (uint32_t i = 0; i < m_swapchain.ImageViews().size(); ++i)
	{
		m_cube_ubo.CreateDescriptorSet(i, m_desc_sets.Get(i));
		m_view_ubo.CreateDescriptorSet(i, m_desc_sets.Get(i));

		m_cube_ubo.UpdateDescriptorSet(g_VkGenerator.Device(), i);
		m_view_ubo.UpdateDescriptorSet(g_VkGenerator.Device(), i);

		for (auto& model : m_render_list)
		{
			model.CreateDescriptorSet(i, m_desc_sets.Get(i), m_sampler.SamplerInstance());
			model.UpdateDescriptorSet(g_VkGenerator.Device(), i);
		}

		m_dir_light_ubo.CreateDescriptorSet(i, m_desc_sets.Get(i));
		m_dir_light_ubo.UpdateDescriptorSet(g_VkGenerator.Device(), i);
	}
}

void VkSponzaDemo::CreateResources()
{
	m_sampler = VkRes::Sampler<vk::Filter::eLinear>(g_VkGenerator.Device(),
	                                                vk::SamplerAddressMode::eClampToEdge,
	                                                0.0f, VK_FALSE,
	                                                0.0f);

	m_cube_ubo = VkRes::UniformBuffer<CubeData, VkRes::EDataUsageFlags::PerFrame>
	(g_VkGenerator.Device(), g_VkGenerator.PhysicalDevice(),
	 m_swapchain.ImageViews().size(), false);

	m_view_ubo = VkRes::UniformBuffer<ViewportData, VkRes::EDataUsageFlags::OnResize>
	(g_VkGenerator.Device(), g_VkGenerator.PhysicalDevice(),
	 m_swapchain.ImageViews().size(), false);

	m_dir_light_ubo = VkRes::UniformBuffer<DirectionalLight, VkRes::EDataUsageFlags::Once>
	(g_VkGenerator.Device(), g_VkGenerator.PhysicalDevice(),
	 m_swapchain.ImageViews().size(), false);
}

bool updated_once = false;

void VkSponzaDemo::UpdateBufferData(uint32_t _image_index, bool _resize)
{
	if (_resize)
	{
		if (m_view_ubo.WantsOnResizeUpdate())
		{
			m_view_ubo.GetData(_image_index).dims = glm::vec2(static_cast<float>(m_swapchain.Extent().width),
			                                                  static_cast<float>(m_swapchain.Extent().height));
			m_view_ubo.Map(g_VkGenerator.Device(), _image_index);
		}
	}
	else
	{
		static auto start_time = std::chrono::high_resolution_clock::now();

		if (m_cube_ubo.WantsPerFrameUpdate())
		{
			auto current_time = std::chrono::high_resolution_clock::now();
			auto time         = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count() *
					0.25f;
			auto dims = m_swapchain.Extent();

			auto m = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 1.0f));
			auto v = glm::lookAt(glm::vec3(2.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			auto p = glm::perspective(glm::radians(45.0f), (float)dims.width / (float)dims.height, 0.1f, 10.0f);
			p[1][1] *= -1;

			m_cube_ubo.GetData(_image_index).mvp   = p * v * m;
			m_cube_ubo.GetData(_image_index).world = m;
			m_cube_ubo.Map(g_VkGenerator.Device(), _image_index);
		}

		if (!updated_once)
		{
			m_dir_light_ubo.GetData(_image_index).direction = glm::vec3(0.0f, 1.0f, 0.0f);
		}
	}
}
