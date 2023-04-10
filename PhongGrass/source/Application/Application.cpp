#include "Application.h"
#include "Time.h"

#include "Graphics/ContentBrowser.h"

#include "Window.h"
#include "Graphics/Renderer.h"
#include "Scene.h"

#include "Components/MeshComponent.h"
#include "Components/Transform.h"

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
	camera.getComponent<Transform>().position = DirectX::XMFLOAT3(5.f, 5.f, -5.f);

	app.scene->setMainCamera(camera);
}

void runApplication()
{
	Time::start();

	while (app.window.isOpen())
	{
		Time::measure();
		app.window.update();

		app.renderer.render();

		app.window.present();
	}
}

void destroyApplication()
{
	app.window.shutdown();
	app.renderer.shutdown();
	app.scene = nullptr;
}
