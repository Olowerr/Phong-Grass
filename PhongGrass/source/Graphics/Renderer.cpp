#include "Renderer.h"
#include "Okay/Okay.h"

#include "ContentBrowser.h"
#include "RenderTexture.h"

#include "Components/Transform.h"
#include "Components/MeshComponent.h"

#include "Application/Scene.h"

#include "imgui/imgui.h"

#include <utility>

namespace Okay
{
	struct PipelineResources
	{
		PipelineResources() = default;
		~PipelineResources();

		// Buffers --- 
		ID3D11Buffer* pCameraBuffer = nullptr;
		ID3D11Buffer* pWorldBuffer = nullptr;
		ID3D11Buffer* pMaterialBuffer = nullptr;
		ID3D11Buffer* pShaderDataBuffer = nullptr;

		ID3D11Buffer* pSkyDataBuffer = nullptr;

		ID3D11Buffer* pLightInfoBuffer = nullptr;
		ID3D11Buffer* pPointLightBuffer = nullptr;
		ID3D11Buffer* pDirLightBuffer = nullptr;
		ID3D11ShaderResourceView* pPointLightSRV = nullptr;
		ID3D11ShaderResourceView* pDirLightSRV = nullptr;
		uint32_t maxPointLights = 0u;
		uint32_t maxDirLights = 0u;

		ID3D11SamplerState* simp = nullptr;

		uint32_t defaultShaderId = INVALID_UINT;
		uint32_t skyboxMeshId = INVALID_UINT;

		// Input layouts
		ID3D11InputLayout* pPosIL = nullptr;
		ID3D11InputLayout* pPosUvNormIL = nullptr;
		ID3D11InputLayout* pPosUvNormJidxWeightIL = nullptr;

		// Vertex shaders
		ID3D11VertexShader* pMeshVS = nullptr;
		ID3D11VertexShader* pSkeletalVS = nullptr;
		ID3D11VertexShader* pSkyBoxVS = nullptr;

		// Rasterizer states
		ID3D11RasterizerState* pWireframeRS = nullptr;
		ID3D11RasterizerState* pNoCullRS = nullptr;

		// Pixel shaders
		ID3D11PixelShader* pSkyBoxPS = nullptr;

		// Depth Stencils
		ID3D11DepthStencilState* pLessEqualDSS = nullptr;
	};

	static PipelineResources pipeline;

	void Renderer::init()
	{
		pDevContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pDevContext->VSSetConstantBuffers(0, 1, &pipeline.pCameraBuffer);
		pDevContext->VSSetConstantBuffers(1, 1, &pipeline.pWorldBuffer);
		pDevContext->VSSetConstantBuffers(2, 1, &pipeline.pMaterialBuffer);
		pDevContext->VSSetConstantBuffers(3, 1, &pipeline.pShaderDataBuffer);
		pDevContext->VSSetConstantBuffers(4, 1, &pipeline.pSkyDataBuffer);
		pDevContext->VSSetConstantBuffers(5, 1, &pipeline.pLightInfoBuffer);

		pDevContext->PSSetConstantBuffers(0, 1, &pipeline.pCameraBuffer);
		pDevContext->PSSetConstantBuffers(1, 1, &pipeline.pWorldBuffer);
		pDevContext->PSSetConstantBuffers(2, 1, &pipeline.pMaterialBuffer);
		pDevContext->PSSetConstantBuffers(3, 1, &pipeline.pShaderDataBuffer);
		pDevContext->PSSetConstantBuffers(4, 1, &pipeline.pSkyDataBuffer);
		pDevContext->PSSetConstantBuffers(5, 1, &pipeline.pLightInfoBuffer);
		pDevContext->PSSetShaderResources(3, 1, &pipeline.pPointLightSRV);
		pDevContext->PSSetShaderResources(4, 1, &pipeline.pDirLightSRV);

		pDevContext->VSSetSamplers(0u, 1u, &pipeline.simp);
		pDevContext->PSSetSamplers(0u, 1u, &pipeline.simp);

		pDevContext->RSSetViewports(1u, &viewport);

		pDevContext->OMSetRenderTargets(1u, pRenderTarget->getRTV(), *pRenderTarget->getDSV());
	}

	Renderer::Renderer()
		:pScene(nullptr), pRenderTarget(nullptr), pDevContext(nullptr), viewport()
	{
	}

	Renderer::Renderer(Ref<RenderTexture> target, Ref<Scene> scene)
		:pScene(nullptr), pRenderTarget(nullptr), pDevContext(nullptr), viewport()
	{
		create(target, scene);
	}

	Renderer::Renderer(uint32_t width, uint32_t height, Ref<Scene> scene)
		:pScene(nullptr), pRenderTarget(nullptr), pDevContext(nullptr), viewport()
	{
		create(width, height, scene);
	}

	Renderer::~Renderer()
	{
		shutdown();
	}

	void Renderer::create(Ref<RenderTexture> target, Ref<Scene> scene)
	{
		shutdown();

		pRenderTarget = target;
		OKAY_ASSERT(pRenderTarget, "RenderTarget was nullptr");
		pScene = scene;
		OKAY_ASSERT(pScene, "Scene was nullptr");

		pDevContext = DX11::get().getDeviceContext();

		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		const DirectX::XMUINT2 dims = pRenderTarget->getDimensions();
		onTargetResize(dims.x, dims.y);
	}

	void Renderer::create(uint32_t width, uint32_t height, Ref<Scene> scene)
	{
		Ref<RenderTexture> target = createRef<RenderTexture>(width, height, RenderTexture::RENDER | RenderTexture::SHADER_READ | RenderTexture::DEPTH);
		create(target, scene);
	}

	void Renderer::shutdown()
	{
		pScene = nullptr;
		pRenderTarget = nullptr;
	}

	void Renderer::setRenderTexture(Ref<RenderTexture> pRenderTexture)
	{
		OKAY_ASSERT(pRenderTexture, "RenderTarget was nullptr");

		pRenderTarget->removeOnResizeCallback(&Renderer::onTargetResize, this);
		pRenderTarget = pRenderTexture;
		pRenderTarget->addOnResizeCallback(&Renderer::onTargetResize, this);

		const DirectX::XMUINT2 dims = pRenderTarget->getDimensions();
		onTargetResize(dims.x, dims.y);
	}

	void Renderer::imGui()
	{
		/*if (!ImGui::Begin("Shaders"))
		{
			ImGui::End();
			return;
		}


		ImGui::End();*/
	}

	void Renderer::render()
	{
		
	}

	void Renderer::onTargetResize(uint32_t width, uint32_t height)
	{
		viewport.Width = (float)width;
		viewport.Height = (float)height;
		pDevContext->RSSetViewports(1u, &viewport);
	}

	void Renderer::updateCameraBuffer(const Entity& cameraEntity)
	{
		
	}
	
	PipelineResources::~PipelineResources()
	{
		DX11_RELEASE(pCameraBuffer);
		DX11_RELEASE(pWorldBuffer);
		DX11_RELEASE(pMaterialBuffer);
		DX11_RELEASE(pShaderDataBuffer);

		DX11_RELEASE(pSkyDataBuffer);

		DX11_RELEASE(pLightInfoBuffer);
		DX11_RELEASE(pPointLightBuffer);
		DX11_RELEASE(pDirLightBuffer);
		DX11_RELEASE(pPointLightSRV);
		DX11_RELEASE(pDirLightSRV);

		DX11_RELEASE(simp);

		DX11_RELEASE(pPosIL);
		DX11_RELEASE(pPosUvNormIL);
		DX11_RELEASE(pPosUvNormJidxWeightIL);

		DX11_RELEASE(pMeshVS);
		DX11_RELEASE(pSkeletalVS);
		DX11_RELEASE(pSkyBoxVS);

		DX11_RELEASE(pWireframeRS);
		DX11_RELEASE(pNoCullRS);

		DX11_RELEASE(pSkyBoxPS);

		DX11_RELEASE(pLessEqualDSS);
	}
}
