#pragma once
#include "Window.h"
#include "UI.h"
#include "Profiler.h"
#include "Shader.h"
#include "InputLayout.h"
#include "Quad.h"
#include "Viewport.h"
#include "ConstantBuffer.h"
#include "PerspectiveCamera.h"
#include "ResourceManager.h"
#include "Timer.h"
#include "Model.h"
#include "PoolAllocator.h"

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define PROFILE_FUNC Profiler TOKENPASTE2(profiler, __LINE__) (__FUNCTION__, [&](ProfileMetrics profileMetrics) {m_ProfileMetrics.push_back(std::move(profileMetrics));})
#define PROFILE_SCOPE(scopeName) Profiler TOKENPASTE2(profiler, __LINE__) (scopeName, [&](ProfileMetrics profileMetrics) {m_ProfileMetrics.push_back(std::move(profileMetrics));})

struct Transform
{
	DirectX::XMMATRIX wvpMatrix;
};

class Application
{
public:
	Application() noexcept;
	~Application() noexcept = default;
	void Run() noexcept;
	void CleanUp();
private:
	void DisplayProfilingResults() noexcept;
	void GetPackagePath() noexcept;
	template<typename T>
	void RenderPoolAllocatorProgressBar(std::unique_ptr<PoolAllocator<T>>& poolAllocator) noexcept;
private:
	bool m_Running;
	std::unique_ptr<Time> m_timer;

	std::vector<std::unique_ptr<Model>> m_pModels;

	std::unique_ptr<UI> m_pImGui;
	std::vector<ProfileMetrics> m_ProfileMetrics;
	std::unique_ptr<VertexShader> m_pVertexShader;
	std::unique_ptr<PixelShader> m_pPixelShader;
	std::unique_ptr<InputLayout> m_pInputLayout;
	std::shared_ptr<Texture2D> m_pBrickTexture;
	std::shared_ptr<Texture2D> m_pThanosTexture;
	std::shared_ptr<Texture2D> m_pBackPackDiffuse;
	std::unique_ptr<Quad> m_pQuad;
	std::unique_ptr<Viewport> m_pViewport;
	std::unique_ptr<PerspectiveCamera> m_pCamera;
	std::unique_ptr<ConstantBuffer> m_pConstantBuffer;
	std::vector<std::shared_ptr<MeshOBJ>> m_pBackPackMeshes;

	bool level1;
};

template<typename T>
void Application::RenderPoolAllocatorProgressBar(std::unique_ptr<PoolAllocator<T>>& poolAllocator) noexcept
{
	ImGui::Begin("Pool Allocator memory usage");
	ImGui::Text("Tag:");
	ImGui::SameLine();
	ImGui::Text(poolAllocator->GetTag());
	static float progress = 0.0f;

	progress = static_cast<float>(poolAllocator->GetByteUsage() / static_cast<float>(poolAllocator->GetByteCapacity()));
	progress = 1.0f - progress;

	ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("Bytes free.");

	char buf[64];
#pragma warning(disable:4996)
	sprintf(buf, "%d/%d", (int)((poolAllocator->GetEntityCapacity() - poolAllocator->GetEntityUsage())), (int)(poolAllocator->GetEntityCapacity()));
	ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f), buf);
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("Entity chunks available.");
	ImGui::End();
}