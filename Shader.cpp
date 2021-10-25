#include "pch.h"
#include "Shader.h"
#include "Graphics.h"
#include "DXDebug.h"
#include "Utility.h"

Shader::Shader() noexcept
	: m_pShaderBlob{nullptr}
{

}

VertexShader::VertexShader(const std::string& fileName) noexcept
	: m_pVertexShader{nullptr}
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) | defined (_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
	Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob = nullptr;
	HRESULT hrTemp = D3DCompileFromFile(ConvertStringToWstring(fileName).c_str(),
										nullptr,
										D3D_COMPILE_STANDARD_FILE_INCLUDE,
										"vs_main",
										"vs_5_0",
										flags,
										0u,
										&m_pShaderBlob,
										&pErrorBlob);
	if (!SUCCEEDED(hrTemp))
	{
		if (pErrorBlob)
		{
			std::cout << (char*)pErrorBlob->GetBufferPointer();
		}
		std::cout << "HRESULT was not successful";
		assert(false);
	}
#else
	D3DCompileFromFile(ConvertStringToWstring(fileName).c_str(),
					   nullptr,
					   D3D_COMPILE_STANDARD_FILE_INCLUDE,
					   "vs_main",
					   "vs_5_0",
					   flags,
					   0u,
					   &m_pShaderBlob,
					   nullptr);
#endif
	HR_I(Graphics::GetDevice()->CreateVertexShader(m_pShaderBlob->GetBufferPointer(), m_pShaderBlob->GetBufferSize(), nullptr, &m_pVertexShader));
}

void VertexShader::Bind() noexcept
{
	CHECK_STD(Graphics::GetContext()->VSSetShader(m_pVertexShader.Get(), nullptr, 0u));
}

PixelShader::PixelShader(const std::string& fileName) noexcept
	: m_pPixelShader{nullptr}
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) | defined (_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
	Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob = nullptr;
	HRESULT hrTemp = D3DCompileFromFile(ConvertStringToWstring(fileName).c_str(),
										nullptr,
										D3D_COMPILE_STANDARD_FILE_INCLUDE,
										"ps_main",
										"ps_5_0",
										flags,
										0u,
										&m_pShaderBlob,
										&pErrorBlob);
	if (!SUCCEEDED(hrTemp))
	{
		if (pErrorBlob)
		{
			std::cout << (char*)pErrorBlob->GetBufferPointer();
		}
		std::cout << "HRESULT was not successful";
		assert(false);
	}
#else
	D3DCompileFromFile(ConvertStringToWstring(fileName).c_str(),
					   nullptr,
					   D3D_COMPILE_STANDARD_FILE_INCLUDE,
					   "ps_main",
					   "ps_5_0",
					   flags,
					   0u,
					   &m_pShaderBlob,
					   nullptr);
#endif
	HR_I(Graphics::GetDevice()->CreatePixelShader(m_pShaderBlob->GetBufferPointer(), m_pShaderBlob->GetBufferSize(), nullptr, &m_pPixelShader));
}

void PixelShader::Bind() noexcept
{
	CHECK_STD(Graphics::GetContext()->PSSetShader(m_pPixelShader.Get(), nullptr, 0u));
}
