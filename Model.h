#pragma once
#include "Texture.h"
#include "Mesh.h"

class Model
{
public:
	Model(std::string, std::string, DirectX::XMFLOAT3, DirectX::XMFLOAT3) noexcept;
	~Model() noexcept = default;

	void Update(float);
	void BindTexture();
	void BindMesh(uint32_t);

	size_t GetNrOfMeshes();
	uint64_t GetNrOfMeshIndices(uint32_t); //Returns the number of indices of the given mesh.
	DirectX::XMMATRIX GetWMatrix();
	std::vector<std::shared_ptr<MeshOBJ>> m_pMeshes;
private:
	std::shared_ptr<Texture2D> m_pTexture;

	DirectX::XMFLOAT3 m_scale;
	DirectX::XMFLOAT3 m_pos;

	DirectX::XMMATRIX m_wMatrix;
};