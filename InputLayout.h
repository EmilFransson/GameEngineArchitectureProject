#pragma once
class InputLayout
{
public:
	InputLayout(const Microsoft::WRL::ComPtr<ID3DBlob>& vertexShaderBlob) noexcept;
	~InputLayout() noexcept = default;
	void Bind() noexcept;
private:
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout;
};