#include "pch.h"
#include "ConstantBuffer.h"
#include "Graphics.h"
#include "DXDebug.h"
#include "Utility.h"

ConstantBuffer::ConstantBuffer(const UINT byteWidth, const UINT structureByteStride, void* pData) noexcept
{
	D3D11_BUFFER_DESC constantBufferDescriptor{};
	constantBufferDescriptor.ByteWidth = byteWidth;
	constantBufferDescriptor.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	constantBufferDescriptor.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	constantBufferDescriptor.MiscFlags = 0u;
	constantBufferDescriptor.StructureByteStride = structureByteStride;
	
	D3D11_SUBRESOURCE_DATA subResourceData{};
	subResourceData.pSysMem = pData;

	HR_I(Graphics::GetDevice()->CreateBuffer(&constantBufferDescriptor, &subResourceData, &m_pConstantBuffer));
}

void ConstantBuffer::BindToVertexShader(const uint8_t slot) noexcept
{
	CHECK_STD(Graphics::GetContext()->VSSetConstantBuffers(slot, 1u, m_pConstantBuffer.GetAddressOf()));
}
