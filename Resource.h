#pragma once

class Resource
{
public:
	Resource(const std::string& assetName, const std::string& type) noexcept;
	virtual ~Resource() noexcept = default;
	[[nodiscard]] constexpr std::string GetName() const noexcept { return m_AssetName; }
	[[nodiscard]] constexpr std::string GetType() const noexcept { return m_Type; }
protected:
	GUID m_GUID;
	std::string m_AssetName;
	std::string m_Type;
};