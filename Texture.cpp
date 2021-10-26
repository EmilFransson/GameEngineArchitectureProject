#include "pch.h"
#include "Texture.h"
#include "Graphics.h"
#include "DXDebug.h"
#include "ResourceManager.h"
#include "Utility.h"

Texture::Texture(const uint32_t width, const uint32_t height, std::string fileName, std::string type) noexcept
	:Resource{fileName, type},
	m_pShaderResourceView{ nullptr },
	m_Width{ width },
	m_Height{ height },
	m_FileName{ fileName }
{
}

Texture2D::Texture2D(const uint32_t width, const uint32_t height, const uint32_t rowPitch, void* pData, const DXGI_FORMAT textureFormat, std::string fileName, std::string type) noexcept
	: Texture{ width, height, fileName, type },
	m_pTexture2D{ nullptr },
	m_pSamplerState{ nullptr }
{
	D3D11_TEXTURE2D_DESC textureDescriptor{};
	textureDescriptor.Width = width;
	textureDescriptor.Height = height;
	textureDescriptor.MipLevels = 1u;
	textureDescriptor.ArraySize = 1u;
	textureDescriptor.Format = textureFormat;
	textureDescriptor.SampleDesc.Count = 1u;
	textureDescriptor.SampleDesc.Quality = 0u;
	textureDescriptor.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT; // Potentially temporary.
	textureDescriptor.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDescriptor.CPUAccessFlags = 0u;
	textureDescriptor.MiscFlags = 0u;
	D3D11_SUBRESOURCE_DATA subResourceData{};
	subResourceData.pSysMem = pData;
	subResourceData.SysMemPitch = rowPitch;
	HR_I(Graphics::GetDevice()->CreateTexture2D(&textureDescriptor, &subResourceData, &m_pTexture2D));

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = textureDescriptor.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1u;
	SRVDesc.Texture2D.MostDetailedMip = 0u;

	HR_I(Graphics::GetDevice()->CreateShaderResourceView(m_pTexture2D.Get(), &SRVDesc, &m_pShaderResourceView));

	D3D11_SAMPLER_DESC samplerDescriptor{};
	samplerDescriptor.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDescriptor.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescriptor.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescriptor.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescriptor.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDescriptor.MinLOD = 0.0f;
	samplerDescriptor.MaxLOD = D3D11_FLOAT32_MAX;
	HR_I(Graphics::GetDevice()->CreateSamplerState(&samplerDescriptor, &m_pSamplerState));
}

std::shared_ptr<Texture2D> Texture2D::Create(const std::string& filePath) noexcept
{
	return ResourceManager::Get()->Load<Texture2D>(ResourceManager::Get()->ConvertGUIDToPair(ResourceManager::Get()->m_FileNameToGUIDMap[filePath]));
}

//Only supports PS as of now [Emil F]
void Texture2D::BindAsShaderResource(const uint8_t slot) noexcept
{
	std::lock_guard<std::mutex> lock(ResourceManager::Get()->m_GUIDToMutexMap[ResourceManager::Get()->ConvertGUIDToPair(ResourceManager::Get()->m_FileNameToGUIDMap[m_FileName])]);
	CHECK_STD(Graphics::GetContext()->PSSetShaderResources(slot, 1u, m_pShaderResourceView.GetAddressOf()));
	CHECK_STD(Graphics::GetContext()->PSSetSamplers(slot, 1u, m_pSamplerState.GetAddressOf()));
}
