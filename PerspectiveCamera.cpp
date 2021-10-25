#include "pch.h"
#include "PerspectiveCamera.h"
#include "Window.h"

PerspectiveCamera::PerspectiveCamera() noexcept :
	m_pos{ 0.0f, 0.0f, -5.0f },
	m_rot{ 0.0f, 0.0f, 0.0f },
	m_forward{ 0.0f, 0.0f, 1.0f },
	m_up{ 0.0f, 1.0f, 0.0f },
	m_right{ 1.0f, 0.0f, 0.0f },
	m_speed{ 20.0f },
	m_yaw{ 0.0f },
	m_pitch{ 0.0f }
{
	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&m_pos), DirectX::XMLoadFloat3(&m_forward), DirectX::XMLoadFloat3(&m_up));
	DirectX::XMStoreFloat4x4(&m_ViewMatrix, viewMatrix);

	float aspectRatio = static_cast<float>(Window::GetWidth()) / static_cast<float>(Window::GetHeight());
	DirectX::XMMATRIX perspectiveMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(45.0f), aspectRatio, 0.1f, 1000.0f);
	DirectX::XMStoreFloat4x4(&m_PerspectiveMatrix, perspectiveMatrix);
	DirectX::XMStoreFloat4x4(&m_ViewPerspectiveMatrix, viewMatrix * perspectiveMatrix);
}

void PerspectiveCamera::Update(float deltaTime)
{
	HandleInput(deltaTime);

	DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0);
	DirectX::XMVECTOR lookAt = DirectX::XMVector3Normalize(DirectX::XMVector3TransformCoord(DefaultForward, rotMatrix));

	DirectX::XMMATRIX rotateYMatrix = DirectX::XMMatrixRotationY(m_yaw);
	DirectX::XMStoreFloat3(&m_right, DirectX::XMVector3TransformCoord(DefaultRight, rotateYMatrix));
	//DirectX::XMStoreFloat3(&m_up, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&m_up), rotateYMatrix));
	DirectX::XMStoreFloat3(&m_forward, DirectX::XMVector3TransformCoord(DefaultForward, rotateYMatrix));

	lookAt = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&m_pos), lookAt);

	//Update view matrix.
	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&m_pos), lookAt, DirectX::XMLoadFloat3(&m_up));
	DirectX::XMStoreFloat4x4(&m_ViewMatrix, viewMatrix);

	DirectX::XMStoreFloat4x4(&m_ViewPerspectiveMatrix, viewMatrix * DirectX::XMLoadFloat4x4(&m_PerspectiveMatrix));
}

void PerspectiveCamera::HandleInput(float deltaTime)
{
	std::vector<int> keyMap = Window::Get().GetKeyMap();
	m_yaw -= Window::Get().GetDeltaMouseX() * m_lookSpeed * deltaTime;

	m_pitch -= Window::Get().GetDeltaMouseY() * m_lookSpeed * deltaTime;
	if (m_pitch > DirectX::XM_PIDIV2 - 0.1f)
	{
		m_pitch = DirectX::XM_PIDIV2 - 0.1f;
	}
	if (m_pitch < -DirectX::XM_PIDIV2 + 0.1f)
	{
		m_pitch = -DirectX::XM_PIDIV2 + 0.1f;
	}

	for (auto key : keyMap)
	{
		switch (key)
		{
		case 87: //w
		{
			m_pos.x += (deltaTime * m_forward.x * m_speed);
			
			m_pos.z += (deltaTime * m_forward.z * m_speed);
			break;
		}
		case 65: //a
		{
			m_pos.x -= (deltaTime * m_right.x * m_speed);
			
			m_pos.z -= (deltaTime * m_right.z * m_speed);
			break;
		}
		case 83: //s
		{
			m_pos.x -= (deltaTime * m_forward.x * m_speed);
			
			m_pos.z -= (deltaTime * m_forward.z * m_speed);
			break;
		}
		case 68: //d
		{
			m_pos.x += (deltaTime * m_right.x * m_speed);
			
			m_pos.z += (deltaTime * m_right.z * m_speed);
			break;
		}
		case 32: //Space
		{

			m_pos.y += (deltaTime * m_up.y * m_speed);

			break;
		}
		case 16: //Space
		{

			m_pos.y -= (deltaTime * m_up.y * m_speed);

			break;
		}
		default:
		{
			break;
		}
		}
	}
}