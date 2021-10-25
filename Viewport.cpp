#include "pch.h"
#include "Viewport.h"
#include "Graphics.h"
#include "DXDebug.h"
#include "Utility.h"
#include "Window.h"

//Hard coded for this project:
Viewport::Viewport() noexcept
{
	m_Viewport.TopLeftX = 0.0f;
	m_Viewport.TopLeftY = 0.0f;
	m_Viewport.Width = static_cast<FLOAT>(Window::GetWidth());
	m_Viewport.Height = static_cast<FLOAT>(Window::GetHeight());
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;
}

void Viewport::Bind() noexcept
{
	CHECK_STD(Graphics::GetContext()->RSSetViewports(1u, &m_Viewport));
}
