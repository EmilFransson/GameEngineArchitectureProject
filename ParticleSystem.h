#pragma once
#include "Model.h"

class ParticleSystem
{
public:
	ParticleSystem() noexcept = default;
	ParticleSystem(DirectX::XMFLOAT3 center, float width, size_t numberOfParticles) noexcept :
		m_center{ center }, m_width{ width }, m_nrOfParticles{ numberOfParticles }
	{

	}
	~ParticleSystem() = default;

	void AddParticles();
	std::vector<Model*> m_pParticles;
private:
	DirectX::XMFLOAT3 m_center;
	float m_width;
	size_t m_nrOfParticles;
};