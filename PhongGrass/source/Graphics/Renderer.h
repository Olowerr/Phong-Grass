#pragma once
#include <vector>

#include "Components/DirectionalLight.h"	
#include "Components/Camera.h"			
#include "Components/SkyLight.h"			

#include "Application/Entity.h"


namespace Okay
{
	class RenderTexture;
	class Scene;

	struct MeshComponent;
	struct Transform;


	class Renderer
	{
	public:
		

		Renderer();
		Renderer(Ref<RenderTexture> target, Ref<Scene> scene);
		Renderer(uint32_t targetWidth, uint32_t targetHeight, Ref<Scene> scene);
		~Renderer();
		

		void create(Ref<RenderTexture> target, Ref<Scene> scene);
		void create(uint32_t targetWidth, uint32_t targetHeight, Ref<Scene> scene);
		void shutdown();

		void setRenderTexture(Ref<RenderTexture> pRenderTexture);
		inline Ref<RenderTexture> getRenderTexture();
		inline const Ref<RenderTexture> getRenderTexture() const;

		inline void setScene(Ref<Scene> pScene);
		inline void setCustomCamera(Entity camera = Entity());

		void render();

		void imGui();

	private:
		void init();

		Ref<Scene> pScene;
		Ref<RenderTexture> pRenderTarget;
		Entity customCamera;

		ID3D11DeviceContext* pDevContext;
		D3D11_VIEWPORT viewport;

		void onTargetResize(uint32_t width, uint32_t height);

	private: 
		void updateCameraBuffer(const Entity& cameraEntity); 
	};

	inline Ref<RenderTexture> Renderer::getRenderTexture()				{ return pRenderTarget; }
	inline const Ref<RenderTexture> Renderer::getRenderTexture() const	{ return pRenderTarget; }

	inline void Renderer::setScene(Ref<Scene> scene)
	{ 
		OKAY_ASSERT(scene, "Scene was nullptr");
		pScene = scene;
	}

	inline void Renderer::setCustomCamera(Entity camera) { customCamera = camera; }
}