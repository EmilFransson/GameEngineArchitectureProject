#include "pch.h"
#include "InputLayout.h"
#include "Graphics.h"
#include "DXDebug.h"
#include "Utility.h"

InputLayout::InputLayout(const Microsoft::WRL::ComPtr<ID3DBlob>& vertexShaderBlob) noexcept
{
	D3D11_INPUT_ELEMENT_DESC inputElementDescriptor[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"NORMAL",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA },
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HR_I(Graphics::GetDevice()->CreateInputLayout(inputElementDescriptor,
												  ARRAYSIZE(inputElementDescriptor),
												  vertexShaderBlob->GetBufferPointer(),
												  vertexShaderBlob->GetBufferSize(),
												  &m_pInputLayout));
}

void InputLayout::Bind() noexcept
{
	CHECK_STD(Graphics::GetContext()->IASetInputLayout(m_pInputLayout.Get()));
}
