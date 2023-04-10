#pragma once
#include "Window.h"
#include "Graphics/Renderer.h"
#include "Scene.h"


namespace Okay
{
	class Application
	{
	public:
		Application(const wchar_t* appName, uint32_t width = 1600u, uint32_t height = 900u);
		virtual ~Application();

		void start();
		void update();
		void end();

		void run();

	protected:
		Window window;
		Ref<Renderer> renderer;
		Ref<Scene> scene;

		std::vector<Ref<Renderer>> registeredRenderers;
	};
}
