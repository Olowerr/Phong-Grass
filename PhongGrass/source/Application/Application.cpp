#include "Application.h"
#include "DirectX/DX11.h"

#include "Components/MeshComponent.h"

#include "Time.h"

namespace Okay
{
	Application::Application(const wchar_t* appName, uint32_t width, uint32_t height)
		:window(width, height, appName, RenderTexture::RENDER | RenderTexture::DEPTH | RenderTexture::SHADER_READ | RenderTexture::SHADER_WRITE)
	{
		scene = createRef<Scene>();
		renderer = createRef<Renderer>(window.getRenderTexture(), scene);

	}

	Application::~Application()
	{
	}

	void Application::start()
	{

	}

	void Application::update()
	{

	}

	void Application::end()
	{

	}

	void Application::run()
	{
		start();
		Time::start();

		while (window.isOpen())
		{
			Time::measure();
			window.update();

			update();

			renderer->render();

			window.present();
		}

		end();
	}

}
