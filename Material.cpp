#include "pch.h"
#include "Material.h"

Material::Material(const PackageTool::SMaterial& material, const std::string& assetName) noexcept
	: Resource{assetName},
	  m_FileName{material.fileName},
	  m_Name{material.name},
	  m_Ns{material.Ns},
	  m_Ni{material.Ni},
	  m_D{material.d},
	  m_Illum{material.illum},
	  m_Map_Ka{material.map_Ka},
	  m_Map_Kd{material.map_Kd},
	  m_Map_Ks{material.map_Ks},
	  m_Map_Ns{material.map_Ns},
	  m_Map_D{material.map_d},
	  m_Map_Bump{material.map_bump}
{
	m_AmbientColor.x = material.Ka.X;
	m_AmbientColor.y = material.Ka.Y;
	m_AmbientColor.z = material.Ka.Z;
	m_DiffuseColor.x = material.Kd.X;
	m_DiffuseColor.y = material.Kd.Y;
	m_DiffuseColor.z = material.Kd.Z;
	m_SpecularColor.x = material.Ks.X;
	m_SpecularColor.y = material.Ks.Y;
	m_SpecularColor.z = material.Ks.Z;
}