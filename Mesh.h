#pragma once
#include "Resource.h"
#include "Mesh.h"
#include "Material.h"
class Mesh : public Resource
{
public:
	Mesh(const std::string& assetName, std::string type) noexcept;
	virtual ~Mesh() noexcept override = default;
	virtual void BindInternals(const uint8_t slot = 0u) noexcept = 0;
protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;
};

class MeshOBJ : public Mesh 
{
public:
	MeshOBJ(const std::vector<objl::Vertex>& vertices, const std::vector<unsigned int> indices, const std::shared_ptr<Material>& pMaterial = nullptr, std::string assetName = "?", std::string fileName = "?", std::string type = "?") noexcept;
	virtual ~MeshOBJ() noexcept override = default;
	virtual void BindInternals(const uint8_t slot = 0u) noexcept override;
	[[nodiscard]] static std::vector<std::shared_ptr<MeshOBJ>> Create(std::string& fileName) noexcept;
	[[nodiscard]] const uint64_t GetNrOfIndices() const noexcept;
	std::string m_FileName; //File it comes from.
	std::string m_AssetName;
private:
	uint64_t m_NrOfIndices;
	UINT m_Strides;
	std::shared_ptr<Material> m_pMaterial;
	bool m_HasMaterial;
};