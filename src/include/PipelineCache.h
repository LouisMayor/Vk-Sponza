#pragma once

#include <fstream>
#include <filesystem>
#include <memory>

extern VkGen::VkGenerator g_VkGenerator;

namespace VkRes
{
	class PipelineCache
	{
	public:

		PipelineCache() = default;

		void Create(vk::Device _device, std::string _dir, std::string _name)
		{
			const std::string file = _dir + _name + ".bin";
			std::fstream      file_read(file, std::ios_base::in | std::ios_base::binary);

			size_t                  cache_size = 0;
			std::unique_ptr<char[]> cache_data = nullptr;

			if (file_read)
			{
				file_read.seekg(0, std::ios_base::end);

				cache_size = static_cast<size_t>(file_read.tellg());

				file_read.clear();
				file_read.seekg(0, std::ios_base::beg);

				cache_data = std::make_unique<char[]>(cache_size);
				file_read.read(cache_data.get(), cache_size);
				if (file_read.gcount() != cache_size)
				{
					std::runtime_error("failed to read");
				}

				file_read.close();
			}

			bool invalid_cache = false;
			if (cache_data != nullptr)
			{
				/* Cache Header Layout:
				 * https://vulkan.lunarg.com/doc/view/1.0.26.0/linux/vkspec.chunked/ch09s06.html
				 * |------------------------------------------------------------------------------------------------|
				 * | Offset |	Size		  |	Description															|
				 * |------------------------------------------------------------------------------------------------|
				 * | 0		|	4			  |	Length in bytes of cache header										|
				 * |------------------------------------------------------------------------------------------------|
				 * | 4		|	4			  |	Cache Header version - vk::PipelineCacheHeaderVersion				|
				 * |------------------------------------------------------------------------------------------------|
				 * | 8		|	4			  |	Vendor ID - vk::PhysicalDeviceProperties::vendorID					|
				 * |------------------------------------------------------------------------------------------------|
				 * | 12		|	4			  |	Device ID - vk::PhysicalDeviceProperties::deviceID					|
				 * |------------------------------------------------------------------------------------------------|
				 * | 16		|	VK_UUID_SIZE  |	Pipeline Cache ID - vk::PhysicalDeviceProperties::pipelineCacheUUID |
				 * |------------------------------------------------------------------------------------------------|
				 */

				const uint32_t size = sizeof uint32_t;

				uint32_t header_length                     = 0;
				uint32_t header_version                    = 0;
				uint32_t vendor_id                         = 0;
				uint32_t device_id                         = 0;
				uint8_t  pipeline_cache_UUID[VK_UUID_SIZE] = {0};

				void* const retrieved_data = cache_data.get();

				std::memcpy(&header_length, static_cast<uint8_t*>(retrieved_data), size);
				std::memcpy(&header_version, static_cast<uint8_t*>(retrieved_data) + 4, size);
				std::memcpy(&vendor_id, static_cast<uint8_t*>(retrieved_data) + 8, size);
				std::memcpy(&device_id, static_cast<uint8_t*>(retrieved_data) + 12, size);
				std::memcpy(pipeline_cache_UUID, static_cast<uint8_t*>(retrieved_data) + 16, VK_UUID_SIZE);

				const auto info = g_VkGenerator.DeviceInfomation().device_properties;

				if (header_length <= 0)
				{
					invalid_cache = true;
					g_Logger.Error("header_length <= 0");
				}

				if (header_version != static_cast<uint32_t>(vk::PipelineCacheHeaderVersion::eOne))
				{
					invalid_cache = true;
					g_Logger.Error("header_version != vk::PipelineCacheHeaderVersion::eOne");
				}

				if (vendor_id != info.vendorID)
				{
					invalid_cache = true;
					g_Logger.Error("vendor_id != this vendor id");
				}

				if (device_id != info.deviceID)
				{
					invalid_cache = true;
					g_Logger.Error("device_id != this device id");
				}

				if (std::memcmp(pipeline_cache_UUID, info.pipelineCacheUUID, sizeof(pipeline_cache_UUID)) != 0)
				{
					invalid_cache = true;
					g_Logger.Error("pipeline cache UUID mismatch");
				}

				if (invalid_cache)
				{
					cache_data.reset();
					cache_size = 0;

					const std::string device_message =
							"Device info from cache file " +
							VkGen::VendorIDToString(vendor_id) +
							" - ID " + std::to_string(device_id) +
							" use http://vulkan.gpuinfo.org/ to find the device using the ID";

					g_Logger.Info(device_message);
					g_Logger.Info(
						"Couldn't read pipeline cache. Deleting this cache file and will create a new cache for this device.");
					std::filesystem::remove(file);
				}
			}

			const vk::PipelineCacheCreateInfo create_info =
			{
				{},
				cache_size,
				invalid_cache ?
					nullptr :
					cache_data.get()
			};

			const auto result = _device.createPipelineCache(&create_info, nullptr, &m_pipeline_cache);
			assert(("Failed to create pipeline cache", result == vk::Result::eSuccess));
		}

		bool HasFile(std::string _dir, std::string _name) const
		{
			const std::string file = _dir + _name + ".bin";
			return std::filesystem::exists(file);
		}

		void WriteToFile(vk::Device _device, std::string _dir, std::string _name)
		{
			const std::string       file  = _dir + _name + ".bin";
			std::ios_base::openmode flags = std::ios_base::out | std::ios_base::binary;
			std::ofstream           cache_file(file, flags);

			const auto result_data = _device.getPipelineCacheData(m_pipeline_cache);
			assert(("Failed to get pipeline cache data", result_data.size() > 0));

			cache_file.write(reinterpret_cast<const char*>(result_data.data()), result_data.size());
			cache_file.close();
		}

		void Destroy(vk::Device _device)
		{
			if (m_pipeline_cache != nullptr)
			{
				_device.destroyPipelineCache(m_pipeline_cache);
				m_pipeline_cache = nullptr;
			}
		}

		[[nodiscard]] vk::PipelineCache& Cache()
		{
			return m_pipeline_cache;
		}

		[[nodiscard]] bool HasCache() const
		{
			return m_pipeline_cache != nullptr;
		}

	private:

		vk::PipelineCache m_pipeline_cache;
	};
}
