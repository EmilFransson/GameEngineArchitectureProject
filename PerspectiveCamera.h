#pragma once
class PerspectiveCamera
{
public:
	PerspectiveCamera() noexcept;
	~PerspectiveCamera() noexcept = default;

	void Update(float);
	[[nodiscard]] constexpr DirectX::XMFLOAT4X4& GetViewProjectionMatrix() noexcept { return m_ViewPerspectiveMatrix; }
	[[nodiscard]] constexpr DirectX::XMFLOAT3 GetPos() noexcept { return m_pos; }
private:
	void HandleInput(float);
	float m_speed;
	float m_lookSpeed = 2.0f;
	DirectX::XMFLOAT4X4 m_ViewMatrix;
	DirectX::XMFLOAT4X4 m_PerspectiveMatrix;
	DirectX::XMFLOAT4X4 m_ViewPerspectiveMatrix;

	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT3 m_rot;
	DirectX::XMFLOAT3 m_forward;
	DirectX::XMFLOAT3 m_up;
	DirectX::XMFLOAT3 m_right;

	DirectX::XMVECTOR DefaultForward{ 0.0f, 0.0f, 1.0f };
	DirectX::XMVECTOR DefaultRight{ 1.0f, 0.0f, 0.0f };

	float m_yaw;
	float m_pitch;
};