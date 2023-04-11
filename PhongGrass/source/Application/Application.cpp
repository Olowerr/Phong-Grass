#include "Application.h"
#include "Time.h"
#include "Input/Input.h"

#include "Graphics/ContentBrowser.h"

#include "Window.h"
#include "Graphics/Renderer.h"
#include "Scene.h"

#include "Components/MeshComponent.h"
#include "Components/Transform.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

using namespace Okay;

struct ApplicationData
{
	Window window;
	Renderer renderer;
	Ref<Scene> scene;

	ApplicationData() = default;
	~ApplicationData() = default;
};

static ApplicationData app;

void updateCamera();
void imGuiStart();
void imGuiNewFrame();
void imGuiRender();
void imGuiDestroy();

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

	content.importFile(RESOURCES_PATH "meshes/smoothCube.fbx");
	app.renderer.initGrass(content.getAmount<Mesh>() - 1u);
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

	const float baseSpeed = 5.f;

	const float frameSpeed = Time::getDT() * baseSpeed * (Input::isKeyDown(Key::L_SHIFT) ? 10.f : 1.f);

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