#include "Application.h"
#include "Time.h"
#include "Input/Input.h"

#include "Graphics/ContentBrowser.h"

#include "Window.h"
#include "Graphics/Renderer.h"
#include "Scene.h"

#include "Components/MeshComponent.h"
#include "Components/Transform.h"

#include "poisson-disk-sampling/poisson_disk_sampling.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

using namespace Okay;

struct ApplicationData
{
	Window window;
	Renderer renderer;
	Ref<Scene> scene;

	float cameraSpeed = 5.f;
	std::vector<std::pair<uint32_t, std::string>> grassMeshes;
	uint32_t currentSelected = 0u;

	float grassDistRadius = 0.1f;
	std::array<float, 2> min{-20.f, -20.f}, max{20.f, 20.f};
	uint32_t seed = 0u;
	uint32_t numBlades = 0u;

	ApplicationData() = default;
	~ApplicationData() = default;
};

static ApplicationData app;
namespace DX = DirectX;

void updateCamera();
void imGuiStart();
void imGuiNewFrame();
void imGuiRender();
void imGuiDestroy();

void generateGrassTransforms(std::vector<DX::XMFLOAT4X4>& result)
{
	srand(app.seed);

	std::vector<std::array<float, 2>> points = std::move(thinks::PoissonDiskSampling(app.grassDistRadius, app.min, app.max, 30u, app.seed));

	app.numBlades = (uint32_t)points.size();
	result.resize(points.size());
	for (size_t i = 0; i < points.size(); i++)
	{
		DX::XMStoreFloat4x4(&result[i],
			DX::XMMatrixTranspose(DX::XMMatrixRotationY((rand() / (float)RAND_MAX) * DX::XM_2PI) * DX::XMMatrixTranslation(points[i][0], 0.f, points[i][1])));
	}
}

void startApplication(const wchar_t* appName, uint32_t width, uint32_t height)
{
	app.window.create(width, height, appName, RenderTexture::RENDER | RenderTexture::DEPTH | RenderTexture::SHADER_READ);
	app.scene = createRef<Scene>();
	app.renderer.create(app.window.getRenderTexture(), app.scene);

	ContentBrowser& content = ContentBrowser::get();
	content.importFile(RESOURCES_PATH "meshes/cube.fbx");
	content.importFile(RESOURCES_PATH "meshes/DefaultTexture.png");

	Entity entity = app.scene->createEntity();
	entity.addComponent<MeshComponent>();

	Entity camera = app.scene->createEntity();
	camera.addComponent<Camera>();
	camera.getComponent<Transform>().position = DirectX::XMFLOAT3(2.f, 2.f, -5.f);
	app.scene->setMainCamera(camera);


	const uint32_t countB4 = content.getAmount<Mesh>();

	content.importFile(RESOURCES_PATH "meshes/grass1.fbx");
	content.importFile(RESOURCES_PATH "meshes/grass2.fbx");
	content.importFile(RESOURCES_PATH "meshes/grass3.fbx");

	app.grassMeshes.reserve(5);
	app.grassMeshes.emplace_back(std::pair{ countB4,	 "grass1"});
	app.grassMeshes.emplace_back(std::pair{ countB4 + 1, "grass2"});
	app.grassMeshes.emplace_back(std::pair{ countB4 + 2, "grass3"});
	app.renderer.setGrassMeshId(countB4 + 2);
	
	std::vector<DX::XMFLOAT4X4> matrices;
	generateGrassTransforms(matrices);
	app.renderer.initGrass(matrices);
}

void runApplication()
{
	imGuiStart();
	Time::start();

	while (app.window.isOpen())
	{
		Time::measure();
		app.window.update();
		imGuiNewFrame();

		updateCamera();

		if (ImGui::Begin("Phong Grass"))
		{
			ImGui::PushItemWidth(-1.f);

			ImGui::Text("FPS: %.3f", 1.f / Time::getDT());
			ImGui::Text("MS:  %.6f", Time::getDT() * 1000.f);
			
			ImGui::Separator();
			ImGui::Text("Cam Speed:");
			ImGui::SameLine();
			ImGui::DragFloat("##CamSpeed", &app.cameraSpeed, 0.05f, 0.f, 10.f);
			
			ImGui::Separator();
			if (ImGui::BeginCombo("##select", app.grassMeshes[app.currentSelected].second.c_str()))
			{
				for (uint32_t i = 0; i < (uint32_t)app.grassMeshes.size(); i++)
				{
					if (ImGui::Selectable(app.grassMeshes[i].second.c_str(), i == app.currentSelected))
						app.renderer.setGrassMeshId(app.grassMeshes[app.currentSelected = i].first);
				}

				ImGui::EndCombo();
			}

			ImGui::Separator();

			ImGui::Text("Num blades: %u", app.numBlades);

			ImGui::Text("Radius:     ");
			ImGui::SameLine();
			ImGui::DragFloat("##radius", &app.grassDistRadius, 0.0001f, 0.f, 10.f, "%.4f");
			
			ImGui::Text("Min:        ");
			ImGui::SameLine();
			ImGui::DragFloat2("##min", app.min.data(), 0.1f);

			ImGui::Text("Max:        ");
			ImGui::SameLine();
			ImGui::DragFloat2("##max", app.max.data(), 0.1f);

			ImGui::Text("Seed:       ");
			ImGui::SameLine();
			ImGui::InputInt("##seed", (int*)&app.seed, 1, 10);

			if (ImGui::Button("Generate grass"))
			{
				std::vector<DX::XMFLOAT4X4> matrices;
				generateGrassTransforms(matrices);
				app.renderer.initGrass(matrices);
			}


			ImGui::PopItemWidth();
		}
		ImGui::End();

		app.renderer.imGui();
		app.renderer.render();
		imGuiRender();

		app.window.present();
	}
}

void destroyApplication()
{
	app.window.shutdown();
	app.renderer.shutdown();
	app.scene = nullptr;

	imGuiDestroy();
}

void updateCamera()
{
	using namespace Okay;
	using namespace DirectX;

	const float frameSpeed = Time::getDT() * app.cameraSpeed * (Input::isKeyDown(Key::L_SHIFT) ? 10.f : 1.f);

	const float xInput = (float)Input::isKeyDown(Key::D) - (float)Input::isKeyDown(Key::A);
	const float yInput = (float)Input::isKeyDown(Key::SPACE) - (float)Input::isKeyDown(Key::L_CTRL);
	const float zInput = (float)Input::isKeyDown(Key::W) - (float)Input::isKeyDown(Key::S);
	const float xRot = (float)Input::isKeyDown(Key::DOWN) - (float)Input::isKeyDown(Key::UP);
	const float yRot = (float)Input::isKeyDown(Key::RIGHT) - (float)Input::isKeyDown(Key::LEFT);

	Transform& tra = app.scene->getMainCamera().getComponent<Transform>();
	const XMVECTOR fwd = tra.forwardXM();
	const XMVECTOR right = tra.rightXM();

	XMVECTOR pos = XMLoadFloat3(&tra.position);

	XMStoreFloat3(&tra.position, pos + (right * xInput + fwd * zInput) * frameSpeed);
	tra.position.y += yInput * frameSpeed;

	tra.rotation.x += xRot * 3.f * Time::getDT();
	tra.rotation.y += yRot * 3.f * Time::getDT();
}

void imGuiStart()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui_ImplWin32_Init(app.window.getHWnd());
	ImGui_ImplDX11_Init(DX11::get().getDevice(), DX11::get().getDeviceContext());
}

void imGuiNewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void imGuiRender()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}

void imGuiDestroy()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
