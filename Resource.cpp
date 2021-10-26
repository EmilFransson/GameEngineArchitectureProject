#include "pch.h"
#include "Resource.h"

Resource::Resource(const std::string& assetName, const std::string& type) noexcept
	: m_GUID{ 0 },
	  m_AssetName{ assetName },
	  m_Type{ type }
{
}
