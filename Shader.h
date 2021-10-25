#pragma once
class Shader
{
public:
	Shader() noexcept;
	virtual ~Shader() noexcept = default;
	virtual void Bind() noexcept = 0;
protected:
	Microsoft::WRL::ComPtr<ID3DBlob> m_pShaderBlob;
};

class VertexShader : public Shader
{
public:
	VertexShader(const std::string& fileName) noexcept;
	virtual ~VertexShader() noexcept override = default;
	virtual void Bind() noexcept override;
	[[nodiscard]] const Microsoft::WRL::ComPtr<ID3DBlob>& GetVertexShaderBlob() const noexcept { return m_pShaderBlob; }
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader;
};

class PixelShader : public Shader
{
public:
	PixelShader(const std::string& fileName) noexcept;
	virtual ~PixelShader() noexcept override = default;
	virtual void Bind() noexcept override;
private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixelShader;
};