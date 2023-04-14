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

	// Grass global settings
	std::array<float, 2> min{-40.f, -40.f}, max{40.f, 40.f};
	uint32_t seed = 0u;
	uint32_t numBlades = 0u; // Doesn't control, only shows the value

	// Current settings
	float grassDistRadius = 0.6f;
	GPUGrassData grassShaderData{};
	int currentSelected[2]{ 0,0 };


	ApplicationData() = default;
	~ApplicationData() = default;
};

struct Settings
{
	float bladeDist = 0u;
	float tessExponent = 1.f;

	static const int NUM_DIST_VALUES = 3;
	static const int NUM_EXPO_VALUES = 3;
	static inline float bladesDistanceValues[NUM_DIST_VALUES]{ 0.5f, 0.25f, 0.1f };
	static inline float grassExpoTestValues[NUM_EXPO_VALUES]{ 1.f, 1.f / 3.f, 3.f };
};

static ApplicationData app;
static Settings settings[Settings::NUM_DIST_VALUES * Settings::NUM_EXPO_VALUES]{};


namespace DX = DirectX;

void updateCamera();
void imGuiStart();
void imGuiNewFrame();
void imGuiRender();
void imGuiDestroy();
void runStaticGrassTests(uint32_t meshId);

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

void applySettings(uint32_t currentSettingIdx = INVALID_UINT)
{
#ifndef DIST
	for (uint32_t g = 0; g < Settings::NUM_DIST_VALUES; g++)
	{
		for (uint32_t e = 0; e < Settings::NUM_EXPO_VALUES; e++)
		{
			settings[(g * Settings::NUM_DIST_VALUES + e)].bladeDist = Settings::bladesDistanceValues[g];
			settings[(g * Settings::NUM_DIST_VALUES + e)].tessExponent = Settings::grassExpoTestValues[e];
		}
	}
#endif

	uint32_t idx = currentSettingIdx == INVALID_UINT ? (app.currentSelected[0] * Settings::NUM_DIST_VALUES + app.currentSelected[1]) : currentSettingIdx;
	const Settings& newSettings = settings[idx];

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

	content.importFile(RESOURCES_PATH "meshes/phongGrass1.fbx");
	app.renderer.setGrassMeshId(content.getAmount<Mesh>() - 1u);


	app.grassShaderData.maxAppliedDistance = 15.f;
	app.grassShaderData.maxTessFactor = 5.f;
	app.grassShaderData.tessFactorExponent = 1.f;

	app.currentSelected[0] = app.currentSelected[0] = 0;
	for (uint32_t g = 0; g < Settings::NUM_DIST_VALUES; g++)
	{
		for (uint32_t e = 0; e < Settings::NUM_EXPO_VALUES; e++)
		{
			settings[(g * Settings::NUM_DIST_VALUES + e)].bladeDist = Settings::bladesDistanceValues[g];
			settings[(g * Settings::NUM_DIST_VALUES + e)].tessExponent = Settings::grassExpoTestValues[e];
		}
	}
	applySettings();
}

void writeSettingsToFile(std::ofstream& writer, uint32_t settingsIdx, bool writeExponent)
{
	writer << "Test: " << settingsIdx << " | Number of blades : " << app.numBlades <<
		" | Dist: " << settings[settingsIdx].bladeDist;

	if (writeExponent)
		writer << " | Exponent: " << settings[settingsIdx].tessExponent;

	writer << "\n";
}

void runApplication()
{
#ifndef DIST
	imGuiStart();
#endif // !DIST


#ifdef DIST
	app.window.setFullscreen(true);

	static const float TEST_DURATION = 5.f; // Seconds
	static const float DELAY_DURATION = 10.f; // Seconds
	static const uint32_t NUM_TESTS = Settings::NUM_DIST_VALUES * Settings::NUM_EXPO_VALUES;
	float timer = 0.f;
	bool delay = true;

	uint32_t currentTest = 0u;
	uint32_t numFrames = 0u;
	
	std::ofstream writer;
	writer.open("Results/PhongGrass/results_0.txt", std::ofstream::trunc);

	writeSettingsToFile(writer, currentTest, true);
#endif

	Time::start();
	while (app.window.isOpen())
	{
		Time::measure();
		app.window.update();
#ifndef DIST
		imGuiNewFrame();
#endif // DIST

		static bool fullscreen = false;
		if (Input::isKeyPressed(Key::F))
			app.window.setFullscreen(fullscreen = !fullscreen);


#ifdef DIST

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

				if (++currentTest >= NUM_TESTS)
					break;

				writer.close();
				writer.open("Results/PhongGrass/results_" + std::to_string(currentTest) + ".txt", std::ofstream::trunc);

				writeSettingsToFile(writer, currentTest, true);
				applySettings(currentTest);

				Time::start();
			}
			else if (numFrames > 1u)
			{
				writer << dt * 1000.f << "\n";
			}
		}
		

#else
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
			ImGui::DragInt2("##sett", app.currentSelected, 0.05f, 0, 2);
			//app.currentSelected[0] = std::clamp(app.currentSelected[0], 0, 2);
			//app.currentSelected[1] = std::clamp(app.currentSelected[1], 0, 2);
			
			if (ImGui::Button("Update"))
				applySettings();
			

			ImGui::PopItemWidth();
		}
		ImGui::End();
		app.renderer.imGui();
#endif

		app.renderer.render();

#ifndef DIST
		imGuiRender();
#endif // DIST

		app.window.present();
	}

	app.window.setFullscreen(false);

#ifdef DIST
	writer.close();
#endif
}

void destroyApplication()
{
	app.window.shutdown();
	app.renderer.shutdown();
	app.scene = nullptr;

#ifndef DIST
	imGuiDestroy();
#endif
}

void runStaticGrassTests(uint32_t meshId, const std::string& resultNamePrefix)
{
	app.window.setFullscreen(true);

	static const float TEST_DURATION = 5.f; // Seconds
	static const float DELAY_DURATION = 10.f; // Seconds
	static const uint32_t NUM_TESTS = 3u;
	float timer = 0.f;
	bool delay = true;

	uint32_t currentTest = 0u;
	uint32_t numFrames = 0u;

	std::ofstream writer;
	writer.open("Results/StaticGrass/" + resultNamePrefix + "_results_0.txt", std::ofstream::trunc);

	writeSettingsToFile(writer, currentTest, false);

	Time::start();
	while (app.window.isOpen())
	{
		Time::measure();
		app.window.update();

		static bool fullscreen = false;
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

				if (++currentTest >= NUM_TESTS)
					break;

				writer.close();
				writer.open("Results/StaticGrass/" + resultNamePrefix + "_results_" + std::to_string(currentTest) + ".txt", std::ofstream::trunc);

				writeSettingsToFile(writer, currentTest, false);
				
				app.grassDistRadius = settings[currentTest].bladeDist;


				Time::start();
			}
			else if (numFrames > 1u)
			{
				writer << dt * 1000.f << "\n";
			}
		}

		app.renderer.render();

		app.window.present();
	}

	app.window.setFullscreen(false);
	writer.close();
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
