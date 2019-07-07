#pragma once

#include "Settings.h"
#include <memory>

class SponzaDemoSettings : public Settings
{
public:
	explicit SponzaDemoSettings(bool _use_msaa          = false,
	                      int  _msaa_sample_count = 2)
	{
		use_msaa     = _use_msaa;
		sample_level = _msaa_sample_count;
	}

	~SponzaDemoSettings()
	{}

	static SponzaDemoSettings* Instance();

private:
	static std::unique_ptr<SponzaDemoSettings> m_instance;
};
