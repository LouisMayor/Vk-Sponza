#pragma once

#include <vulkan/vulkan.hpp>
#include "Event.hpp"

class Settings
{
public:

	enum class SettingUpdateFlags : unsigned int
	{
		None = 1 << 0,
		SwapchainRecreation = 1 << 1,
		DescriptorsRecreation = 1 << 2,
	};

	explicit Settings(bool _use_msaa          = false,
	                  int  _msaa_sample_count = 2) : use_msaa(_use_msaa),
	                                                 sample_level(_msaa_sample_count)
	{}

	virtual ~Settings()
	{ }

	// Sets enable/disable multi-sampling anti aliasing
	void SetMSAA(bool);

	// Sets multi-sampling anti aliasing sample count
	void SetSampleCount(int);

	vk::SampleCountFlagBits GetSampleCount() const;

	SettingUpdateFlags Updated(bool);

	bool use_msaa     = false;
	int  sample_level = 2;

	Event<bool>                    OnToggleMSAA;
	Event<vk::SampleCountFlagBits> OnSampleCountUpdated;

protected:
	SettingUpdateFlags m_updated = SettingUpdateFlags::None;
};

constexpr Settings::SettingUpdateFlags operator|(Settings::SettingUpdateFlags a, Settings::SettingUpdateFlags b)
{
	return static_cast<Settings::SettingUpdateFlags>(static_cast<int>(a) | static_cast<int>(b));
}

constexpr Settings::SettingUpdateFlags operator|=(Settings::SettingUpdateFlags a, Settings::SettingUpdateFlags b)
{
	return static_cast<Settings::SettingUpdateFlags>(static_cast<int>(a) | static_cast<int>(b));
}

constexpr bool operator&(Settings::SettingUpdateFlags a, Settings::SettingUpdateFlags b)
{
	return static_cast<int>(a) & static_cast<int>(b);
}