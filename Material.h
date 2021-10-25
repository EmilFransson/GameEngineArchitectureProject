#pragma once
#include "Resource.h"
#include "PackageTool.h"
class Material : public Resource
{
public:
	Material(const PackageTool::SMaterial& material, const std::string& assetName) noexcept;
	virtual ~Material() noexcept override = default;
private:
	std::string m_FileName; // ?
	std::string m_Name;
	DirectX::XMFLOAT3 m_AmbientColor;
	DirectX::XMFLOAT3 m_DiffuseColor;
	DirectX::XMFLOAT3 m_SpecularColor;
	float m_Ns;
	float m_Ni;
	float m_D;
	int m_Illum;
	std::string m_Map_Ka;
	std::string m_Map_Kd;
	std::string m_Map_Ks;
	std::string m_Map_Ns;
	std::string m_Map_D;
	std::string m_Map_Bump;
};
