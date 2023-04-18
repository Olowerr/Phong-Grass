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
	Entity floor;
	uint32_t phongGrassMeshId;
	uint32_t phongGrassMesh1Id;
	uint32_t phongGrassMesh2Id;
	uint32_t phongGrassMesh22Id;
	uint32_t lowGrassMeshId;
	uint32_t highGrassMeshId;

	// Grass global settings
	std::array<float, 2> min{-40.f, -40.f}, max{40.f, 40.f};
	uint32_t seed = 0u;
	uint32_t numBlades = 0u; // Doesn't control, only shows the value

	// Current settings
	float grassDistRadius = 0.6f;
	GPUGrassData grassShaderData{};


	ApplicationData() = default;
	~ApplicationData() = default;
};

struct Settings
{
	float bladeDist = 0u;
	float tessExponent = 1.f;

	static const int NUM_DIST_VALUES = 3;
	static const int NUM_EXPO_VALUES = 3;
	static inline float bladesDistanceValues[NUM_DIST_VALUES]{ 0.5f, 0.2f, 0.1f };
	static inline float grassExpoTestValues[NUM_EXPO_VALUES]{ 1.f, 1.f / 3.f, 3.f };
};

static ApplicationData app;
static const uint32_t NUM_SETTINGS = Settings::NUM_DIST_VALUES * Settings::NUM_EXPO_VALUES;
static Settings settings[NUM_SETTINGS]{};


namespace DX = DirectX;

void updateCamera();
void imGuiStart();
void imGuiNewFrame();
void imGuiRender();
void imGuiDestroy();
void runGrassTests(uint32_t meshId, const std::string& resultNamePrefix, bool phongGrass);

void updateFloor()
{
	Transform& tra = app.floor.getComponent<Transform>();

	const float xDiff = app.max[0] - app.min[0];
	const float zDiff = app.max[1] - app.min[1];
	tra.position = DX::XMFLOAT3(xDiff * 0.5f + app.min[0], 0.f, zDiff * 0.5f + app.min[1]);
	tra.scale = DX::XMFLOAT3(xDiff, 1.f, zDiff);
}

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

void updateSettingsArray()
{
	for (uint32_t g = 0; g < Settings::NUM_DIST_VALUES; g++)
	{
		for (uint32_t e = 0; e < Settings::NUM_EXPO_VALUES; e++)
		{
			settings[(g * Settings::NUM_DIST_VALUES + e)].bladeDist = Settings::bladesDistanceValues[g];
			settings[(g * Settings::NUM_DIST_VALUES + e)].tessExponent = Settings::grassExpoTestValues[e];
		}
	}
}

void applySettings(uint32_t settingsIdx)
{
	const Settings& newSettings = settings[settingsIdx];

	app.grassShaderData.tessFactorExponent = newSettings.tessExponent;
	app.grassDistRadius = newSettings.bladeDist;

	std::vector<DX::XMFLOAT4X4> matrices;
	generateGrassTransforms(matrices);
	app.renderer.initGrass(matrices);

	app.renderer.setGrassTessData(app.grassShaderData);
}

void startApplication(const wchar_t* appName, uint32_t width, uint32_t height)
{
	app.window.create(width, height, appName, RenderTexture::RENDER | RenderTexture::DEPTH | RenderTexture::SHADER_READ);
	app.scene = createRef<Scene>();
	app.renderer.create(app.window.getRenderTexture(), app.scene);

	ContentBrowser& content = ContentBrowser::get();
	content.importFile(RESOURCES_PATH "textures/DefaultTexture.png");
	content.importFile(RESOURCES_PATH "textures/groundTex.png");
	content.importFile(RESOURCES_PATH "meshes/floor.fbx");
	Material& floorMat = content.addAsset<Material>();
	floorMat.setBaseColour(content.getAssetID<Texture>("groundTex"));
	floorMat.setName("floorMat");

	app.floor = app.scene->createEntity();
	app.floor.addComponent<MeshComponent>(content.getAssetID<Mesh>("floor"), content.getAssetID<Material>("floorMat"));
	updateFloor();

	Entity camera = app.scene->createEntity();
	camera.addComponent<Camera>();
	camera.getComponent<Transform>().position = DirectX::XMFLOAT3(2.f, 2.f, -5.f);
	app.scene->setMainCamera(camera);

	content.importFile(RESOURCES_PATH "meshes/gob.obj");
	Entity gob = app.scene->createEntity();
	gob.addComponent<MeshComponent>(content.getAmount<Mesh>() - 1u, 0u);

	const uint32_t amountPreImport = content.getAmount<Mesh>();

	content.importFile(RESOURCES_PATH "meshes/phongGrass.fbx");
	content.importFile(RESOURCES_PATH "meshes/phongGrass1.fbx");
	content.importFile(RESOURCES_PATH "meshes/phongGrass2.fbx");
	content.importFile(RESOURCES_PATH "meshes/phongGrass22.fbx");
	content.importFile(RESOURCES_PATH "meshes/lowGrass.fbx");
	content.importFile(RESOURCES_PATH "meshes/highGrass.fbx");

	app.phongGrassMeshId = amountPreImport;
	app.phongGrassMesh1Id = amountPreImport + 1u;
	app.phongGrassMesh2Id = amountPreImport + 2u;
	app.phongGrassMesh22Id = amountPreImport + 3u;
	app.lowGrassMeshId = amountPreImport + 4u;
	app.highGrassMeshId = amountPreImport + 5u;

	app.renderer.setGrassMeshId(app.phongGrassMeshId);


	app.grassShaderData.maxAppliedDistance = 15.f;
	app.grassShaderData.maxTessFactor = 5.f;
	app.grassShaderData.tessFactorExponent = 1.f;

	updateSettingsArray();
}

void writeSettingsToFile(std::ofstream& writer, uint32_t settingsIdx, bool writeExponent)
{
	writer << "SettingID: " << settingsIdx << " | Number of blades : " << app.numBlades <<
		" | Dist: " << settings[settingsIdx].bladeDist;

	if (writeExponent)
		writer << " | Exponent: " << settings[settingsIdx].tessExponent;

	writer << "\n";
}

void runPerformanceTests()
{
	app.window.setFullscreen(true);

	runGrassTests(app.phongGrassMeshId, "PhongGrass", true);
	runGrassTests(app.lowGrassMeshId, "LowQualityGrass", false);
	runGrassTests(app.highGrassMeshId, "HighQualityGrass", false);

	app.window.setFullscreen(false);
}

/*
	SWITCH TO PhongGrass1 ENTIRELY, FIX lowGrass/highGrass
	SWITCH TO PhongGrass1 ENTIRELY, FIX lowGrass/highGrass
	SWITCH TO PhongGrass1 ENTIRELY, FIX lowGrass/highGrass
	SWITCH TO PhongGrass1 ENTIRELY, FIX lowGrass/highGrass
	SWITCH TO PhongGrass1 ENTIRELY, FIX lowGrass/highGrass
	SWITCH TO PhongGrass1 ENTIRELY, FIX lowGrass/highGrass
	SWITCH TO PhongGrass1 ENTIRELY, FIX lowGrass/highGrass
	SWITCH TO PhongGrass1 ENTIRELY, FIX lowGrass/highGrass
	SWITCH TO PhongGrass1 ENTIRELY, FIX lowGrass/highGrass
	SWITCH TO PhongGrass1 ENTIRELY, FIX lowGrass/highGrass
	SWITCH TO PhongGrass1 ENTIRELY, FIX lowGrass/highGrass
	SWITCH TO PhongGrass1 ENTIRELY, FIX lowGrass/highGrass
	SWITCH TO PhongGrass1 ENTIRELY, FIX lowGrass/highGrass
	SWITCH TO PhongGrass1 ENTIRELY, FIX lowGrass/highGrass
*/

void runEditorApplication()
{
	imGuiStart();

	bool phongTess = true;
	int meshId = (int)app.phongGrassMeshId;
	int currSettings2D[2]{};
	applySettings(0u);
	app.renderer.setGrassMeshId(app.phongGrassMeshId);

	Time::start();
	while (app.window.isOpen())
	{
		Time::measure();
		app.window.update();
		imGuiNewFrame();

		static bool fullscreen = false;
		if (Input::isKeyPressed(Key::F))
			app.window.setFullscreen(fullscreen = !fullscreen);
	
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

			ImGui::Text("Min:        ");
			ImGui::SameLine();
			ImGui::DragFloat("##min", app.min.data(), 0.05f);
			app.min[1] = app.min[0];

			ImGui::Text("Max:        ");
			ImGui::SameLine();
			ImGui::DragFloat("##max", app.max.data(), 0.05f);
			app.max[1] = app.max[0];

			ImGui::Text("Seed:       ");
			ImGui::SameLine();
			ImGui::InputInt("##seed", (int*)&app.seed, 1, 10);

			if (ImGui::Button("Generate grass"))
			{
				std::vector<DX::XMFLOAT4X4> matrices;
				generateGrassTransforms(matrices);
				app.renderer.initGrass(matrices);
				updateFloor();
			}

			ImGui::Separator();

			ImGui::Text("Num blades: %u", app.numBlades);

			ImGui::Text("Max Tess:");
			ImGui::SameLine();
			if (ImGui::DragFloat("##maxTess", &app.grassShaderData.maxTessFactor, 0.01f, 0.f, 10.f))
				app.renderer.setGrassTessData(app.grassShaderData);

			ImGui::Text("Max Dist:");
			ImGui::SameLine();
			if (ImGui::DragFloat("##maxDist", &app.grassShaderData.maxAppliedDistance, 0.05f, 0.f, 100.f))
				app.renderer.setGrassTessData(app.grassShaderData);
			
			ImGui::Separator();

			ImGui::Text("DistValues:");
			ImGui::SameLine();
			ImGui::DragFloat3("##dists", Settings::bladesDistanceValues, 0.0001f, 0.f, 4.f, "%.4f");

			ImGui::Text("Exponents:");
			ImGui::SameLine();
			ImGui::DragFloat3("##exps", Settings::grassExpoTestValues, 0.0001f, 0.f, 10.f, "%.4f");
			
			ImGui::Text("Settings:");
			ImGui::SameLine();
			ImGui::DragInt2("##sett", currSettings2D, 0.05f, 0, 2);
			
			if (ImGui::Button("Update"))
			{
				updateSettingsArray();
				applySettings(currSettings2D[0] * Settings::NUM_DIST_VALUES + currSettings2D[1]);
			}
			
			ImGui::Separator();
			ImGui::Checkbox("Phong", &phongTess);
			ImGui::RadioButton("Phong Blade", &meshId, (int)app.phongGrassMeshId);
			ImGui::RadioButton("Phong Blade1", &meshId, (int)app.phongGrassMesh1Id);
			ImGui::RadioButton("Phong Blade2", &meshId, (int)app.phongGrassMesh2Id);
			ImGui::RadioButton("Phong Blade22", &meshId, (int)app.phongGrassMesh22Id);
			ImGui::RadioButton("Low Blade", &meshId, (int)app.lowGrassMeshId);
			ImGui::RadioButton("High Blade", &meshId, (int)app.highGrassMeshId);

			app.renderer.setGrassMeshId(meshId);

			ImGui::PopItemWidth();
		}
		ImGui::End();
		app.renderer.imGui();

		app.renderer.prepareRender();
		app.renderer.renderObjects();
		if (phongTess)
			app.renderer.renderPhongGrass();
		else
			app.renderer.renderStaticGrass();

		imGuiRender();

		app.window.present();
	}

	app.window.setFullscreen(false);
	imGuiDestroy();
}

void runGrassTests(uint32_t meshId, const std::string& resultNamePrefix, bool phongGrass)
{
	static const float TEST_DURATION = 2.f; // Seconds
	static const float DELAY_DURATION = 6.f; // Seconds
	static bool fullscreen = true;

	float timer = 0.f;
	bool delay = true;

	uint32_t currentTest = 0u;
	uint32_t numFrames = 0u;

	const std::string fileLocation = (phongGrass ? "Results/PhongGrass/" : "Results/StaticGrass/") + resultNamePrefix;
	std::ofstream writer;
	writer.open(fileLocation + "_results_0.txt", std::ofstream::trunc);

	applySettings(currentTest);
	writeSettingsToFile(writer, currentTest, phongGrass);
	app.renderer.setGrassMeshId(meshId);

	Time::start();
	while (app.window.isOpen())
	{
		Time::measure();
		app.window.update();

		if (Input::isKeyPressed(Key::F))
			app.window.setFullscreen(fullscreen = !fullscreen);

		const float dt = Time::getApplicationDT();
		timer += dt;
		if (delay)
		{
			if (timer > DELAY_DURATION)
			{
				timer = 0.f;
				delay = false;
			}
		}
		else
		{
			numFrames++;
			if (timer > TEST_DURATION)
			{
				writer << "Avg: " << (timer * 1000.f) / (float)numFrames << " | NumFrames: " << numFrames << " | Test Duration: " << TEST_DURATION << "\n";
				timer = 0.f;
				numFrames = 0u;
				delay = true;

				currentTest += phongGrass ? 1u : 3u;
				if (currentTest >= NUM_SETTINGS)
					break;

				writer.close();
				writer.open(fileLocation + "_results_" + std::to_string(currentTest / (phongGrass ? 1u : 3u)) + ".txt", std::ofstream::trunc);
				
				applySettings(currentTest);
				writeSettingsToFile(writer, currentTest, phongGrass);

				Time::start();
			}
			else if (numFrames > 1u)
			{
				writer << dt * 1000.f << "\n";
			}
		}

		app.renderer.prepareRender();
		if (phongGrass)
			app.renderer.renderPhongGrass();
		else
			app.renderer.renderStaticGrass();

		app.window.present();
	}

	writer.close();
}

void destroyApplication()
{
	app.window.shutdown();
	app.renderer.shutdown();
	app.scene = nullptr;
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
