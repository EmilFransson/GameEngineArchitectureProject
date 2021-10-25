#include "pch.h"
#include "Resource.h"

Resource::Resource(const std::string& assetName) noexcept
	: m_GUID{ 0 },
	  m_AssetName{ assetName }
{
}
