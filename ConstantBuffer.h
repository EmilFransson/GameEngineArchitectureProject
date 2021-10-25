#pragma once

class ConstantBuffer
{
public:
	ConstantBuffer(const UINT byteWidth, const UINT structureByteStride, void* pData) noexcept;
	~ConstantBuffer() noexcept = default;
	void BindToVertexShader(const uint8_t slot = 0u) noexcept;
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pConstantBuffer;
};