#include "include/SponzaDemoSettings.h"

std::unique_ptr<SponzaDemoSettings> SponzaDemoSettings::m_instance = std::make_unique<SponzaDemoSettings>();

SponzaDemoSettings* SponzaDemoSettings::Instance()
{
	if (m_instance == nullptr)
	{
		m_instance = std::make_unique<SponzaDemoSettings>();
	}

	return m_instance.get();
}
