#include "pch.h"
#include "Application.h"
#include "System.h"
#include "RenderCommand.h"
#include "PackageTool.h"
#include "StackAllocator.h"
#include "ParticleSystem.h"

Application::Application() noexcept
	: m_Running{true}, level1{true}
{
	System::Initialize();
	m_timer = std::make_unique<Time>();

	ResourceManager::Get()->Init();

	StackAllocator::CreateAllocator(100000);

	//Default 1280 x 720 window, see function-parameters for dimensions.
	Window::Initialize(L"GameEngineArchitecture");
	m_pImGui = std::make_unique<UI>();
	ResourceManager::Get()->MapPackageContent();

	m_pVertexShader = std::make_unique<VertexShader>("Shaders/VertexShader.hlsl");
	m_pPixelShader = std::make_unique<PixelShader>("Shaders/PixelShader.hlsl");
	m_pInputLayout = std::make_unique<InputLayout>(m_pVertexShader->GetVertexShaderBlob());
	m_pVertexShader->Bind();
	m_pPixelShader->Bind();
	m_pInputLayout->Bind();
	
	//Create vectors with filenames for stack allocation of objects.
	//Level 1
	m_level1ModelNames.push_back("Cone.obj");
	m_level1ModelNames.push_back("CubeLevel1.obj");
	m_level1ModelNames.push_back("Diamond.obj");
	m_level1ModelNames.push_back("Pyramid.obj");
	m_level1ModelNames.push_back("Sphere.obj");

	m_level1TextureNames.push_back("Blue.jpg");
	m_level1TextureNames.push_back("bricks.png");
	m_level1TextureNames.push_back("Cobblestone.jpg");
	m_level1TextureNames.push_back("Droplets.jpg");

	//Level 2
	m_level2ModelNames.push_back("backpack.obj");
	m_level2ModelNames.push_back("Skull.obj");
	m_level2ModelNames.push_back("Moon_2K.obj");

	m_level2TextureNames.push_back("diffuse.jpg");
	m_level2TextureNames.push_back("Skull.jpg");
	m_level2TextureNames.push_back("Diffuse_2K.png");

	using t_clock = std::chrono::high_resolution_clock;
	std::default_random_engine generator(static_cast<UINT>(t_clock::now().time_since_epoch().count()));

	std::uniform_int_distribution<int> distributionNrOfObjects(5, 10);
	std::uniform_int_distribution<int> distributionModels(0, m_level1ModelNames.size() - 1);
	std::uniform_int_distribution<int> distributionTextures(0, m_level1TextureNames.size() - 1);

	std::uniform_real_distribution<float> distributionPosition(-30.0f, 30.0f);
	std::uniform_real_distribution<float> distributionScale(0.4f, 2.5f);
	size_t nrOfModels = distributionNrOfObjects(generator);
	//Create models 
	for (size_t i = 0; i < nrOfModels; i++)
	{
		float scale = distributionScale(generator);
		m_pModels.push_back(std::make_unique<Model>(
			m_level1ModelNames[distributionModels(generator)],
			m_level1TextureNames[distributionTextures(generator)],
			DirectX::XMFLOAT3(30.0f + distributionPosition(generator), distributionPosition(generator) / 2.0f, distributionPosition(generator)),
			DirectX::XMFLOAT3(scale, scale, scale)
			));
	}
	
	RenderCommand::SetTopolopy(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pViewport = std::make_unique<Viewport>();
	m_pViewport->Bind();

	m_pCamera = std::make_unique<PerspectiveCamera>();
}

void Application::CleanUp()
{
	m_pModels.clear();
	ResourceManager::Get()->CleanUp();
	StackAllocator::GetInstance()->FreeAllMemory();
}

void Application::Run() noexcept
{
	while (m_Running)
	{
		m_timer->Update();
		m_pCamera->Update(static_cast<float>(m_timer->DeltaTime()));

		DirectX::XMFLOAT3 currentPos = m_pCamera->GetPos();

		//Stack variables. Create before the particlesystems.
		//Create the vpMatrix, constant buffer & transform that we use for every drawcall on the stack.
		m_pCurrentViewPerspectiveMatrix = StackAllocator::GetInstance()->New<DirectX::XMMATRIX>(DirectX::XMLoadFloat4x4(&m_pCamera->GetViewProjectionMatrix()));
		m_pConstantBuffer = StackAllocator::GetInstance()->New<ConstantBuffer>();
		m_pTransform = StackAllocator::GetInstance()->New<Transform>();

		std::vector<ParticleSystem*> particleSystems;

		if (level1)
		{
			using t_clock = std::chrono::high_resolution_clock;
			std::default_random_engine generator(static_cast<UINT>(t_clock::now().time_since_epoch().count()));

			float len = sqrt(currentPos.x * currentPos.x + currentPos.y * currentPos.y + currentPos.z * currentPos.z);
			if (len > 50.0f )
			{
				level1 = false;
				//Unload level 1 and load level 2.
				m_pModels.clear();

				std::uniform_int_distribution<int> distributionNrOfObjects(20, 40);
				std::uniform_int_distribution<int> distributionModelsAndTextures(0, std::min(m_level2ModelNames.size(), m_level2TextureNames.size()) - 1);

				std::uniform_real_distribution<float> distributionPosition(-100.0f, 100.0f);
				std::uniform_real_distribution<float> distributionScale(0.4f, 2.5f);

				size_t nrOfModels = distributionNrOfObjects(generator);
				for (size_t i = 0; i < nrOfModels; i++)
				{
					size_t modelIndex = distributionModelsAndTextures(generator);
					float scale = distributionScale(generator);
					m_pModels.push_back(std::make_unique<Model>(
						m_level2ModelNames[modelIndex],
						m_level2TextureNames[modelIndex],
						DirectX::XMFLOAT3(distributionPosition(generator), distributionPosition(generator), distributionPosition(generator)),
						DirectX::XMFLOAT3(scale, scale, scale)
						));
				}
			}
			else
			{
				//Create particles only if in level1.
				std::uniform_int_distribution<int> distributionNrOfObjects(20, 50);

				size_t nrOfParticleSystems = 4;
				for (size_t i = 0; i < nrOfParticleSystems; i++)
				{
					size_t randNum = distributionNrOfObjects(generator);
					particleSystems.push_back(StackAllocator::GetInstance()->New<ParticleSystem>(
						DirectX::XMFLOAT3(10.0f * i, 10.0f, 10.0f),
						1.0f,
						randNum
						));
					if (particleSystems[i])
						particleSystems[i]->AddParticles();
				}
			}
		}
		static const FLOAT color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
		RenderCommand::ClearBackBuffer(color);
		RenderCommand::ClearDepthBuffer();
		RenderCommand::BindBackBuffer();

		static float delta = 0.0f;
		delta += static_cast<float>(m_timer->DeltaTime()) * 10.0f;

		//Rendering starts here.
		Render3D(delta, particleSystems);
		
		//Render 2D ontop.
		Render2D();

		if (!Window::OnUpdate())
		{
			m_pModels.clear();
			m_Running = false;
		}

		//STACK FRAME CLEANUP. DO NOT DO ANYTHING AFTER THIS LINE.
		StackAllocator::GetInstance()->CleanUp();
	}
}

void Application::Render3D(float delta, std::vector<ParticleSystem*>& particleSystems) noexcept
{
	//For every model.
	for (uint32_t i{ 0u }; i < m_pModels.size(); i++)
	{
		m_pModels[i]->Update(static_cast<float>(delta));

		m_pTransform->wvpMatrix = DirectX::XMMatrixTranspose(m_pModels[i]->GetWMatrix() * *m_pCurrentViewPerspectiveMatrix);
		m_pConstantBuffer->UpdateConstantBuffer(static_cast<UINT>(sizeof(Transform)), 0, m_pTransform);
		m_pConstantBuffer->BindToVertexShader();

		//Bind texture for this model.
		m_pModels[i]->BindTexture();
		//Bind the meshes in the model one by one and draw them.
		std::string tempFileName = m_pModels[i]->m_pMeshes[0]->m_FileName;
		for (uint32_t j{ 0u }; j < m_pModels[i]->GetNrOfMeshes(); j++)
		{
			std::lock_guard<std::mutex> lock(ResourceManager::Get()->m_FilenameToMutexMap[tempFileName]);
			if (m_pModels[i]->m_pMeshes[j] == nullptr)
				continue;
			m_pModels[i]->BindMesh(j);
			RenderCommand::DrawIndexed(m_pModels[i]->GetNrOfMeshIndices(j));
			if (j + 1 <= m_pModels[i]->m_pMeshes.size() - 1)
				tempFileName = m_pModels[i]->m_pMeshes[j + 1]->m_FileName;
		}
	}

	//For every particleSystem object
	for (uint32_t i{ 0u }; i < particleSystems.size(); i++)
	{
		if (!particleSystems[i])
			continue;
		//For every model in the particle system.
		for (uint32_t j{ 0u }; j < particleSystems[i]->m_pParticles.size(); j++)
		{
			if (!particleSystems[i]->m_pParticles[j])
				continue;
			particleSystems[i]->m_pParticles[j]->Update(static_cast<float>(delta));

			m_pTransform->wvpMatrix = DirectX::XMMatrixTranspose(particleSystems[i]->m_pParticles[j]->GetWMatrix() * *m_pCurrentViewPerspectiveMatrix);
			m_pConstantBuffer->UpdateConstantBuffer(static_cast<UINT>(sizeof(Transform)), 0, m_pTransform);
			m_pConstantBuffer->BindToVertexShader();

			//Bind texture for this model.
			particleSystems[i]->m_pParticles[j]->BindTexture();
			//Bind the meshes in the model one by one and draw them.
			for (uint32_t k{ 0u }; k < particleSystems[i]->m_pParticles[j]->GetNrOfMeshes(); k++)
			{
				particleSystems[i]->m_pParticles[j]->BindMesh(k);
				RenderCommand::DrawIndexed(particleSystems[i]->m_pParticles[j]->GetNrOfMeshIndices(k));
			}
		}
	}
}

void Application::Render2D() noexcept
{
	UI::Begin();
	// Windows not part of the dock space goes here:

	//...And ends here.
	UI::BeginDockSpace();
	//Windows part of the dock space goes here:
	GetPackagePath();
	RenderStackAllocatorProgressBar();
	RenderPoolAllocatorProgressBar(ResourceManager::m_pTextureAllocator);
	RenderPoolAllocatorProgressBar(ResourceManager::m_pMaterialAllocator);
	RenderPoolAllocatorProgressBar(ResourceManager::m_pMeshOBJAllocator);
	RenderBuddyAllocatorProgressBar();
	ResourceManager::DisplayStateUI();
	//Can be reinstated if we'd need it for any assignment3-profiling.
	//DisplayProfilingResults();
	//...And ends here.
	UI::EndDockSpace();
	//No UI-windows in this part and after!
	UI::End();

	RenderCommand::UnbindRenderTargets();
}

void Application::DisplayProfilingResults() noexcept
{
	ImGui::Begin("Profiling metrics");
	for (auto& metric : m_ProfileMetrics)
	{
		ImGui::Text(std::to_string(metric.Duration).c_str());
		ImGui::SameLine();
		ImGui::Text("ms.");
		ImGui::SameLine();
		ImGui::Text(metric.Name.c_str());
	}
	ImGui::End();
	m_ProfileMetrics.clear();
}

//When a path is entered check if it is a pkg file.
void Application::GetPackagePath() noexcept
{
	ImGui::Begin("Package Path");
	std::unique_ptr<char> path = std::unique_ptr<char>(DBG_NEW char[64](0));
	if (ImGui::InputText("Path: ", path.get(), 64, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AllowTabInput))
	{
		std::string p = path.get();
		std::ifstream pkg;
		if (p.find_last_of(".") == std::string::npos)
		{
			std::cout << "Created .pkg at filepath: " << PackageTool::Package(path.get()) << std::endl;
		}
		else
		{
			std::string extension = p.substr(p.find_last_of("."), p.size() - 1);
			if(extension != ".pkg")
			{
				assert(false);
			}
		}
	}
	ImGui::End();
}

void Application::RenderStackAllocatorProgressBar() noexcept
{
	ImGui::Begin("Stack Allocator memory usage");
	static float progress = 0.0f;

	progress = static_cast<float>(StackAllocator::GetInstance()->GetStackCurrentSize() / static_cast<float>(StackAllocator::GetInstance()->GetStackMaxSize()));
	progress = 1.0f - progress;

	ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("Bytes free.");

	ImGui::End();
}

void Application::RenderBuddyAllocatorProgressBar() noexcept
{
	ImGui::Begin("Buddy Allocator memory usage");
	static float progress = 0.0f;

	progress = static_cast<float>(ResourceManager::buddyAllocator.getUsedMemory()) / static_cast<float>(ResourceManager::buddyAllocator.getMaxMemory());
	progress = 1.0f - progress;

	ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("Bytes free.");

	ImGui::End();
}
