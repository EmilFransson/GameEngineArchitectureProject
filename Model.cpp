#include "pch.h"
#include "Model.h"
#include "ResourceManager.h"

Model::Model(std::string obj, std::string tex, DirectX::XMFLOAT3 startPos, DirectX::XMFLOAT3 startScale) noexcept :
	m_scale{ startScale },
	m_pos{ startPos }
{
	//m_wMatrix = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) * DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&startPos));
	ResourceManager::Get()->tAddJob(obj, nullptr, &m_pMeshes);
	ResourceManager::Get()->tAddJob(tex, &m_pTexture, nullptr);
	//m_pMeshes = MeshOBJ::Create(obj);
	//m_pTexture = Texture2D::Create(tex);
}

void Model::Update(float deltaTime)
{
	m_wMatrix = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&m_scale)) * DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(deltaTime)) * DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&m_pos));
}

void Model::BindTexture()
{
	m_pTexture->BindAsShaderResource();
}

void Model::BindMesh(uint32_t meshIndex)
{
	m_pMeshes[meshIndex]->BindInternals();
}

size_t Model::GetNrOfMeshes()
{
	return m_pMeshes.size();
}

uint64_t Model::GetNrOfMeshIndices(uint32_t meshIndex)
{
	return m_pMeshes[meshIndex]->GetNrOfIndices();
}

DirectX::XMMATRIX Model::GetWMatrix()
{
	return m_wMatrix;
}