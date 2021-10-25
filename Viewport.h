#pragma once

class Viewport
{
public:
	Viewport() noexcept;
	~Viewport() noexcept = default;
	void Bind() noexcept;
private:
	D3D11_VIEWPORT m_Viewport;
};