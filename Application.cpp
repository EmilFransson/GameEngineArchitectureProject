#include "pch.h"
#include "Application.h"
#include "System.h"
#include "RenderCommand.h"
#include "PackageTool.h"

Application::Application() noexcept
	: m_Running{true}, level1{true}
{
	System::Initialize();
	m_timer = std::make_unique<Time>();

	ResourceManager::Get()->Init();
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
	//m_pMesh = MeshOBJ::Create("backpack.obj");
	//m_pMesh->BindInternals();
	//
	//ResourceManager::Get()->tAddJob("Cube.obj", nullptr, &m_pBackPackMeshes);
	//ResourceManager::Get()->tAddJob<Texture2D>("bricks.png", &m_pBrickTexture);
	//m_pBrickTexture = Texture2D::Create("bricks.png");
	// 
	//m_pBrickTexture = Texture2D::Create("bricks.png");
	//m_pThanosTexture = Texture2D::Create("thanos.png");
	
	//Create models 
	m_pModels.push_back(std::make_unique<Model>("CubeLevel1.obj", "bricks.png", DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 2.0f, 1.0f)));
	m_pModels.push_back(std::make_unique<Model>("CubeLevel1.obj", "bricks.png", DirectX::XMFLOAT3(10.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(2.0f, 1.0f, 2.0f)));
	m_pModels.push_back(std::make_unique<Model>("CubeLevel1.obj", "bricks.png", DirectX::XMFLOAT3(-10.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(2.0f, 2.0f, 1.0f)));
	m_pModels.push_back(std::make_unique<Model>("CubeLevel1.obj", "bricks.png", DirectX::XMFLOAT3(20.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 2.0f)));

	RenderCommand::SetTopolopy(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pViewport = std::make_unique<Viewport>();
	m_pViewport->Bind();

	m_pCamera = std::make_unique<PerspectiveCamera>();
}

void Application::CleanUp()
{
	ResourceManager::Get()->CleanUp();
}

void Application::Run() noexcept
{
	while (m_Running)
	{
		m_timer->Update();
		m_pCamera->Update(static_cast<float>(m_timer->DeltaTime()));

		DirectX::XMFLOAT3 currentPos = m_pCamera->GetPos();
		if (level1)
		{
			float len = sqrt(currentPos.x * currentPos.x + currentPos.y * currentPos.y + currentPos.z * currentPos.z);
			if (len > 30.0f )
			{
				level1 = false;
				//Unload level 1 and load level 2.
				m_pModels.clear();

				for (size_t i = 0; i < 20; i++)
				{
					m_pModels.push_back(std::make_unique<Model>("backpack.obj", "diffuse.jpg", DirectX::XMFLOAT3(i * 30.0f, 0.0f, 30.0f), DirectX::XMFLOAT3(i * 1.2f, 2.0f, 1.0f)));
					m_pModels.push_back(std::make_unique<Model>("backpack.obj", "diffuse.jpg", DirectX::XMFLOAT3(i * 25.0f, i * 20.0f, 0.0f), DirectX::XMFLOAT3(1.0f, i * 1.3f, 1.0f)));
					m_pModels.push_back(std::make_unique<Model>("backpack.obj", "diffuse.jpg", DirectX::XMFLOAT3(i * 30.0f, 0.0f, i * 15.0f), DirectX::XMFLOAT3(1.0f, 2.0f, 1.0f)));
					m_pModels.push_back(std::make_unique<Model>("backpack.obj", "diffuse.jpg", DirectX::XMFLOAT3(0.0f, i * 50.0f, i * 10.0f), DirectX::XMFLOAT3(i * 1.1f, 2.0f, 1.0f)));
				}
			}
		}
		static const FLOAT color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
		RenderCommand::ClearBackBuffer(color);
		RenderCommand::ClearDepthBuffer();
		RenderCommand::BindBackBuffer();

		static float delta = 0.0f;
		delta += static_cast<float>(m_timer->DeltaTime()) * 10.0f;
		
		DirectX::XMMATRIX viewPerspectiveMatrix = DirectX::XMLoadFloat4x4(&m_pCamera->GetViewProjectionMatrix());

		//For every model.
		for (uint32_t i{ 0u }; i < m_pModels.size(); i++)
		{
			m_pModels[i]->Update(static_cast<float>(delta));

			static Transform transform{};
			transform.wvpMatrix = DirectX::XMMatrixTranspose(m_pModels[i]->GetWMatrix() * viewPerspectiveMatrix);
			m_pConstantBuffer = std::make_unique<ConstantBuffer>(static_cast<UINT>(sizeof(Transform)), 0, &transform);
			m_pConstantBuffer->BindToVertexShader();

			//Bind texture for this model.
			m_pModels[i]->BindTexture();
			//Bind the meshes in the model one by one and draw them.
			for (uint32_t j{ 0u }; j < m_pModels[i]->GetNrOfMeshes(); j++)
			{
				m_pModels[i]->BindMesh(j);
				RenderCommand::DrawIndexed(m_pModels[i]->GetNrOfMeshIndices(j));
			}
		}

		UI::Begin();
		// Windows not part of the dock space goes here:

		//...And ends here.
		UI::BeginDockSpace();
		//Windows part of the dock space goes here:
		GetPackagePath();

		//Can be reinstated if we'd need it for any assignment3-profiling.
		//DisplayProfilingResults();
		//...And ends here.
		UI::EndDockSpace();
		//No UI-windows in this part and after!
		UI::End();

		RenderCommand::UnbindRenderTargets();

		if (!Window::OnUpdate())
		{
			m_Running = false;
		}
	}
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
	//std::string path;
	std::unique_ptr<char> path = std::unique_ptr<char>(DBG_NEW char[64](0));
	if (ImGui::InputText("Path: ", path.get(), 64, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AllowTabInput))
	{
		std::string p = path.get();
		std::ifstream pkg;
		if (p.find_last_of(".") == std::string::npos)
		{
			//pkg = std::ifstream(path.get(), std::ios::binary);
			//The folder exists
			//if (pkg.is_open())
			//{
				std::cout << "Created .pkg at filepath: " << PackageTool::Package(path.get()) << std::endl;
				//pkg.close();
			//}
			//else
			//{
				//assert(false);
			//}
		}
		else
		{
			std::string extension = p.substr(p.find_last_of("."), p.size() - 1);
			if(extension != ".pkg") //change to .zip
			{
				assert(false);
			}
		}
	}
	ImGui::End();
}