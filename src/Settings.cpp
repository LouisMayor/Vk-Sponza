#include "include/Settings.h"

Settings::SettingUpdateFlags Settings::Updated(bool _has_responded = false)
{
	const SettingUpdateFlags tmp = m_updated;
	if (_has_responded)
	{
		m_updated = SettingUpdateFlags::None;
	}
	return tmp;
}

void Settings::SetMSAA(const bool _value)
{
	const bool tmp = use_msaa;
	use_msaa       = _value;

	if (tmp != use_msaa)
	{
		if (m_updated & SettingUpdateFlags::None)
		{
			m_updated = SettingUpdateFlags::SwapchainRecreation;
		}
		else
		{
			m_updated |= SettingUpdateFlags::SwapchainRecreation;
		}
	}
}

// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
unsigned long SampleCount(unsigned long v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

void Settings::SetSampleCount(const int _value)
{
	const int tmp = sample_level;
	sample_level  = static_cast<int>(SampleCount(_value));



	if (tmp != sample_level)
	{
		if (m_updated & SettingUpdateFlags::None)
		{
			m_updated = SettingUpdateFlags::SwapchainRecreation;
		}
		else
		{
			m_updated |= SettingUpdateFlags::SwapchainRecreation;
		}
	}
}

vk::SampleCountFlagBits Settings::GetSampleCount() const
{
	return vk::SampleCountFlagBits(sample_level);
}
