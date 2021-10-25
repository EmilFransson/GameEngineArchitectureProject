#pragma once

class Resource
{
public:
	Resource(const std::string& assetName) noexcept;
	virtual ~Resource() noexcept = default;
	[[nodiscard]] constexpr std::string GetName() const noexcept { return m_AssetName; }
protected:
	GUID m_GUID;
	std::string m_AssetName;
};