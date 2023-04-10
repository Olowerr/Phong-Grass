#include "Application.h"

#include "Time.h"

#include "Window.h"
#include "Graphics/Renderer.h"
#include "Scene.h"


using namespace Okay;

struct ApplicationData
{
	Window window;
	Ref<Renderer> renderer;
	Ref<Scene> scene;

	ApplicationData() = default;
	~ApplicationData() = default;
};

static ApplicationData app;

void startApplication(const wchar_t* appName, uint32_t width, uint32_t height)
{
	app.window.create(width, height, appName, RenderTexture::RENDER | RenderTexture::DEPTH | RenderTexture::SHADER_READ);
	app.scene = createRef<Scene>();
	app.renderer = createRef<Renderer>(app.window.getRenderTexture(), app.scene);
}

void runApplication()
{
	Time::start();

	while (app.window.isOpen())
	{
		Time::measure();
		app.window.update();

		app.renderer->render();

		app.window.present();
	}
}

void destroyApplication()
{
	app.window.shutdown();
	app.renderer = nullptr;
	app.scene = nullptr;
}
