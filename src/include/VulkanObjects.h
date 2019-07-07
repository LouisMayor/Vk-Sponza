#pragma once

#include "Flags.h"

namespace VkRes
{
	using UINT8 = unsigned char;
	using UINT16 = unsigned short int;
	using UINT32 = unsigned int;
	using UINT64 = long unsigned int;
	using UINT128 = long long unsigned int;

	enum EDataUsageFlags : UINT8
	{
		Once = 1 << 1,
		PerFrame = 1 << 1,
		OnResize = 1 << 2
	};

	constexpr EDataUsageFlags operator|(EDataUsageFlags a, EDataUsageFlags b)
	{
		return static_cast<EDataUsageFlags>(static_cast<int>(a) | static_cast<int>(b));
	}

	using BufferUsageFlags = Flags<EDataUsageFlags, UINT8>;
}

#include "Swapchain.h"
#include "Command.h"
#include "RenderTarget.h"
#include "RenderPass.h"
#include "FrameBuffer.h"
#include "GraphicsPipeline.h"
#include "Shader.h"
#include "Fence.h"
#include "Semaphore.h"
#include "Buffer.h"
#include "Sampler.h"
#include "Texture.h"
#include "DepthBuffer.h"
#include "UniformBuffer.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"
