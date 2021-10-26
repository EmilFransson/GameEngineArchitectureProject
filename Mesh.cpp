#include "pch.h"
#include "Mesh.h"
#include "Graphics.h"
#include "DXDebug.h"
#include "Utility.h"
#include "ResourceManager.h"

Mesh::Mesh(const std::string& assetName, std::string type) noexcept
	: Resource{assetName, type}
{
}

MeshOBJ::MeshOBJ(const std::vector<objl::Vertex>& vertices, const std::vector<unsigned int> indices, const std::shared_ptr<Material>& pMaterial, std::string fileName, std::string type) noexcept
	: Mesh{fileName, type},
	  m_NrOfIndices{ indices.size() }, 
	  m_Strides{ sizeof(objl::Vertex)}, 
	  m_pMaterial{pMaterial}, 
	  m_FileName{ fileName }
{
	D3D11_BUFFER_DESC vertexBufferDescriptor{};
	vertexBufferDescriptor.ByteWidth = sizeof(objl::Vertex) * static_cast<UINT>(vertices.size());
	vertexBufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDescriptor.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDescriptor.CPUAccessFlags = 0u;
	vertexBufferDescriptor.MiscFlags = 0u;
	vertexBufferDescriptor.StructureByteStride = sizeof(objl::Vertex);

	D3D11_SUBRESOURCE_DATA subResourceData{};
	subResourceData.pSysMem = vertices.data();

	HR_I(Graphics::GetDevice()->CreateBuffer(&vertexBufferDescriptor, &subResourceData, &m_pVertexBuffer));

	D3D11_BUFFER_DESC indexBufferDescriptor{};
	indexBufferDescriptor.ByteWidth = sizeof(unsigned int) * static_cast<UINT>(indices.size());
	indexBufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDescriptor.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
	indexBufferDescriptor.CPUAccessFlags = 0u;
	indexBufferDescriptor.MiscFlags = 0u;
	indexBufferDescriptor.StructureByteStride = sizeof(unsigned int);

	D3D11_SUBRESOURCE_DATA subResourceData2{};
	subResourceData2.pSysMem = indices.data();

	HR_I(Graphics::GetDevice()->CreateBuffer(&indexBufferDescriptor, &subResourceData2, &m_pIndexBuffer));

	if (pMaterial != nullptr)
	{
		m_HasMaterial = true;
	}
	else
	{
		m_HasMaterial = false;
	}
}

void MeshOBJ::BindInternals(const uint8_t slot) noexcept
{
	static const UINT offset = 0u;
	std::lock_guard<std::mutex> lock(ResourceManager::Get()->m_FilenameToMutexMap[m_FileName]);
	CHECK_STD(Graphics::GetContext()->IASetVertexBuffers(slot, 1u, m_pVertexBuffer.GetAddressOf(), &m_Strides, &offset));
	CHECK_STD(Graphics::GetContext()->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0u));
}

std::vector<std::shared_ptr<MeshOBJ>> MeshOBJ::Create(std::string& fileName) noexcept
{
	return ResourceManager::Get()->LoadMultiple<MeshOBJ>(fileName);
}

const uint64_t MeshOBJ::GetNrOfIndices() const noexcept
{
	std::lock_guard<std::mutex> lock(ResourceManager::Get()->m_FilenameToMutexMap[m_FileName]);
	return m_NrOfIndices;
}