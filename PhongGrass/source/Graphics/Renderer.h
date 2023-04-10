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

	struct GPURenderData
	{
		DirectX::XMFLOAT4X4 viewProjMatrix;
		DirectX::XMFLOAT3 camPos;
		float pad0;
		DirectX::XMFLOAT3 camDir;
		float pad1;
	};

	struct GPUObjectData
	{
		DirectX::XMFLOAT4X4 worldMatrix;
		DirectX::XMFLOAT2 uvOffset;
		DirectX::XMFLOAT2 uvTiling;
	};

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

		GPURenderData renderData;
		GPUObjectData objectData;

		void onTargetResize(uint32_t width, uint32_t height);
		void updateCameraBuffer(const Entity& cameraEntity); 

	private: // Buffers and stuff
		ID3D11DeviceContext* pDevContext;

		ID3D11Buffer* pRenderDataBuffer = nullptr;
		ID3D11Buffer* pObjectDataBuffer = nullptr;

		ID3D11SamplerState* simp = nullptr;

		ID3D11InputLayout* pPosIL = nullptr;
		ID3D11InputLayout* pPosUvNormIL = nullptr;

		ID3D11VertexShader* pMeshVS = nullptr;
		ID3D11VertexShader* pGrassVS = nullptr;

		ID3D11HullShader* pGrassHS = nullptr;
		ID3D11DomainShader* pGrassDS = nullptr;

		ID3D11RasterizerState* pNoCullRS = nullptr;
		D3D11_VIEWPORT viewport;

		ID3D11PixelShader* pDefaultPS = nullptr;
		ID3D11PixelShader* pGrassPS = nullptr;
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