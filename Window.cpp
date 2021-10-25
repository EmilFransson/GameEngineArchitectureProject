#include "pch.h"
#include "Window.h"
#include "Utility.h"
#include "Graphics.h"
#include "DXDebug.h"
#include "ImGui\imgui.h"
#include "Profiler.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

const Window Window::m_sInstance;
std::wstring Window::m_Title = L"?";
std::wstring Window::m_ClassName = L"GameEngineArchitectureWindowClass";
uint32_t Window::m_Width = 0u;
uint32_t Window::m_Height = 0u;
HWND Window::m_WindowHandle;
MSG Window::m_WindowMessage = { 0 };
bool Window::m_Running = true;
std::vector<int> Window::m_keyMap = { 0 };
float Window::m_deltaMouseX = 0.0f;
float Window::m_deltaMouseY = 0.0f;
int Window::m_oldMouseX = 0;
int Window::m_oldMouseY = 0;
bool Window::m_cursorShow = true;

const Window& Window::Get() noexcept
{
	return m_sInstance;
}

Window::~Window() noexcept
{
	ShutDown();
}

void Window::Initialize(const std::wstring& windowTitle, const uint32_t width, const uint32_t height)
{
	WNDCLASSEX windowClass = { 0 };						//[https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-wndclassexa]
	windowClass.cbSize = sizeof(WNDCLASSEX);			//Size in bytes of structure.
	windowClass.style = CS_OWNDC;						//Every window get its own device context.
	windowClass.lpfnWndProc = HandleMessages;			//Long pointer to function handling the window messages (setup in this case).
	windowClass.cbClsExtra = 0;							//Extra bytes allocated to the window class structure.
	windowClass.cbWndExtra = 0;							//Extra bytes allocated following the window instance.
	windowClass.hInstance = GetModuleHandle(nullptr); 	//Handle to the instance that contains the window procedure for class.
	windowClass.hIcon = nullptr;						//Handle to the class icon.
	windowClass.hCursor = nullptr;						//Handle to the class cursor.
	windowClass.hbrBackground = nullptr;				//Handle to the class background brush.
	windowClass.lpszMenuName = nullptr;					//Pointer to null-terminated character string specifying resource name of class menu.
	windowClass.lpszClassName = m_ClassName.c_str();	//Window class name
	RegisterClassEx(&windowClass);

	/*Calculate the rectangle equivalent to the client region of the window based
	  on the window styles included for the window.*/
	RECT windowRect = { 0 };
	windowRect.left = 0;
	windowRect.right = width + windowRect.left;
	windowRect.top = 0;
	windowRect.bottom = windowRect.top + height;
	AdjustWindowRect(&windowRect, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

	//Calculate dimension-agnostic start position for window:
	int windowStartPositionX = static_cast<int>((GetSystemMetrics(SM_CXSCREEN) *
		(((GetSystemMetrics(SM_CXSCREEN) - width)
			/ (float)GetSystemMetrics(SM_CXSCREEN)) / 2)));

	int windowStartPositionY = static_cast<int>((GetSystemMetrics(SM_CYSCREEN) *
		(((GetSystemMetrics(SM_CYSCREEN) - height)
			/ (float)GetSystemMetrics(SM_CYSCREEN)) / 2)));
																	//[https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexa]
	m_WindowHandle = CreateWindowEx(0,								//Extended window styles (bits).
					 m_ClassName.c_str(),							//The window class name.
					 windowTitle.c_str(),							//The window title.
					 WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,		//The window styles defining the window appearance.
					 windowStartPositionX,							//Start x-position for window.
					 windowStartPositionY,							//Start y-position for window.
					 windowRect.right - windowRect.left,			//Width of window.
					 windowRect.bottom - windowRect.top,			//Height of window.
					 nullptr,										//Handle to window parent.								
					 nullptr,										//Handle to menu.
					 GetModuleHandle(nullptr),						//Handle to the instance that contains the window procedure for class. 
					 nullptr);										//Pointer to value passed to the window through CREATESTRUCT structure.

	m_Title = windowTitle;
	m_Width = width;
	m_Height = height;


	RAWINPUTDEVICE Rid[1];

	Rid[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
	Rid[0].usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
	Rid[0].dwFlags = 0;    // adds mouse and also ignores legacy mouse messages
	Rid[0].hwndTarget = 0;
	RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

	ShowWindow(m_WindowHandle, SW_SHOWNORMAL);
	
	//DirectX initialization:
	Graphics::Initialize(m_Width, m_Height);
}

const uint32_t Window::GetWidth() noexcept
{
	return m_Width;
}

const uint32_t Window::GetHeight() noexcept
{
	return m_Height;
}

std::pair<uint32_t, uint32_t> Window::GetWindowResolution() noexcept
{
	return std::pair<uint32_t, uint32_t>(m_Width, m_Height);
}

const bool Window::OnUpdate()
{
	m_deltaMouseX = 0.0f;
	m_deltaMouseY = 0.0f;
	/*
	if (!m_cursorShow)
	{
		SetCursorPos(GetSystemMetrics(SM_CXSCREEN) / 2, GetSystemMetrics(SM_CYSCREEN) / 2);
	}
	*/
	while (PeekMessage(&m_WindowMessage, nullptr, 0u, 0u, PM_REMOVE))
	{
		TranslateMessage(&m_WindowMessage);
		DispatchMessage(&m_WindowMessage);
	}

	HR_I(Graphics::GetSwapChain()->Present(0u, 0u));
	return m_Running;
}

LRESULT Window::HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_CLOSE:
	{
		m_Running = false;
		break;
	}
	case WM_LBUTTONDOWN:
	{
		break;
	}
	case WM_INPUT:
	{
		UINT dwSize;

		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[dwSize];
		if (lpb == NULL)
		{
			return 0;
		}

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
			OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

		RAWINPUT* raw = (RAWINPUT*)lpb;

		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			/*
			if (raw->data.mouse.usButtonFlags == RI_MOUSE_LEFT_BUTTON_DOWN)
			{
				ShowCursor(!m_cursorShow);
				m_cursorShow = !m_cursorShow;
			}
			*/
			m_deltaMouseX = static_cast<float>(-raw->data.mouse.lLastX);
			m_deltaMouseY = static_cast<float>(-raw->data.mouse.lLastY);
		}
		delete[] lpb;
		
		break;
	}
	case WM_KEYDOWN:
	{
		bool keyFound = false;
		for (size_t i = 0; i < m_keyMap.size(); i++)
		{
			if (wParam == m_keyMap[i])
			{
				keyFound = true;
				break;
			}
		}

		if (!keyFound)
		{
			m_keyMap.push_back(static_cast<int>(wParam));
		}
		break;
	}
	case WM_KEYUP:
	{
		size_t i = 0;
		bool keyFound = false;
		for (i; i < m_keyMap.size(); i++)
		{
			if (wParam == m_keyMap[i])
			{
				keyFound = true;
				break;
			}
		}
		if (keyFound)
		{
			m_keyMap.erase(m_keyMap.begin() + i);
		}

		break;
	}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

std::vector<int> Window::GetKeyMap()
{
	return m_keyMap;
}

float Window::GetDeltaMouseX()
{
	return m_deltaMouseX;
}

float Window::GetDeltaMouseY()
{
	return m_deltaMouseY;
}

void Window::ShutDown()
{
	//BOOL result = UnregisterClass(m_ClassName.c_str(), GetModuleHandle(nullptr));
	//RLS_ASSERT(result != FALSE, "Failed to unregister class.");
}
