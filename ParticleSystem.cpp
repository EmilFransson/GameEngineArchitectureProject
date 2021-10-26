#include "pch.h"
#include "ParticleSystem.h"
#include "StackAllocator.h"

void ParticleSystem::AddParticles()
{
	for (size_t i = 0; i < m_nrOfParticles; i++)
	{
		using t_clock = std::chrono::high_resolution_clock;
		std::default_random_engine generator(static_cast<UINT>(t_clock::now().time_since_epoch().count()));

		std::uniform_real_distribution<float> distributionPos(-m_width, m_width);
		std::uniform_real_distribution<float> distributionScale(0.01f, 0.05f);

		float randScale = distributionScale(generator);

		m_pParticles.push_back(StackAllocator::GetInstance()->New<Model>(
			"Cube.obj",
			"Grey.png",
			DirectX::XMFLOAT3(m_center.x + distributionPos(generator), m_center.y + distributionPos(generator), m_center.z + distributionPos(generator)),
			DirectX::XMFLOAT3(randScale, randScale, randScale)
			));
	}
}
