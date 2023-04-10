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
	void Renderer::init()
	{
		bool result = false;
		HRESULT hr = E_FAIL;
		DX11& dx11 = DX11::get();
		ID3D11Device* pDevice = dx11.getDevice();

		// Buffers
		{
			hr = DX11::createConstantBuffer(&pRenderDataBuffer, nullptr, sizeof(GPURenderData), false);
			OKAY_ASSERT(SUCCEEDED(hr), "Failed creating pRenderDataBuffer");

			hr = DX11::createConstantBuffer(&pObjectDataBuffer, nullptr, sizeof(GPUObjectData), false);
			OKAY_ASSERT(SUCCEEDED(hr), "Failed creating pObjectDataBuffer");
		}

		// Rasterizer states
		{
			D3D11_RASTERIZER_DESC rsDesc{};
			rsDesc.FillMode = D3D11_FILL_SOLID;
			rsDesc.CullMode = D3D11_CULL_NONE;
			rsDesc.FrontCounterClockwise = FALSE;
			rsDesc.DepthBias = 0;
			rsDesc.SlopeScaledDepthBias = 0.0f;
			rsDesc.DepthBiasClamp = 0.0f;
			rsDesc.DepthClipEnable = TRUE;
			rsDesc.ScissorEnable = FALSE;
			rsDesc.MultisampleEnable = FALSE;
			rsDesc.AntialiasedLineEnable = FALSE;
			hr = pDevice->CreateRasterizerState(&rsDesc, &pNoCullRS);
			OKAY_ASSERT(SUCCEEDED(hr), "Failed creating noCullRS");
		}


		// Basic linear sampler
		{
			D3D11_SAMPLER_DESC simpDesc{};
			simpDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			simpDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			simpDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			simpDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			simpDesc.MinLOD = -FLT_MAX;
			simpDesc.MaxLOD = FLT_MAX;
			simpDesc.MipLODBias = 0.f;
			simpDesc.MaxAnisotropy = 1u;
			simpDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			hr = pDevice->CreateSamplerState(&simpDesc, &simp);
			OKAY_ASSERT(SUCCEEDED(hr), "Failed creating sampler");
		}


		// Input Layouts & Shaders
		{
			D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[3] = {
				{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"UV",			0, DXGI_FORMAT_R32G32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
			};

			std::string shaderData;

			result = DX11::createShader(SHADER_PATH "MeshVS.hlsl", &pMeshVS, &shaderData);
			OKAY_ASSERT(result, "Failed creating vertex shader");

			hr = pDevice->CreateInputLayout(inputLayoutDesc, 3u, shaderData.c_str(), shaderData.length(), &pPosUvNormIL);
			OKAY_ASSERT(SUCCEEDED(hr), "Failed creating input layout");

#if GRASS
			result = DX11::createShader(SHADER_PATH "GrassVS.hlsl", &pGrassVS, &shaderData);
			OKAY_ASSERT(result, "Failed creating grass vertex shader");

			hr = pDevice->CreateInputLayout(inputLayoutDesc, 1u, shaderData.c_str(), shaderData.length(), &pPosIL);
			OKAY_ASSERT(SUCCEEDED(hr), "Failed creating pos only input layout");

			result = DX11::createShader(SHADER_PATH "GrassHS.hlsl", &pGrassHS);
			OKAY_ASSERT(result, "Failed creating grass hull shhader");

			result = DX11::createShader(SHADER_PATH "GrassDS.hlsl", &pGrassDS);
			OKAY_ASSERT(result, "Failed creating grass domain shhader");
			
			result = DX11::createShader(SHADER_PATH "GrassPS.hlsl", &pGrassPS);
			OKAY_ASSERT(result, "Failed creating grass pixel shader");
#endif

			result = DX11::createShader(SHADER_PATH "DefaultPS.hlsl", &pDefaultPS);
			OKAY_ASSERT(result, "Failed creating Default pixel shader");

		}

		pDevContext->VSSetConstantBuffers(0, 1, &pRenderDataBuffer);
		pDevContext->VSSetConstantBuffers(1, 1, &pObjectDataBuffer);
		pDevContext->VSSetSamplers(0u, 1u, &simp);
		
		//pDevContext->HSSetConstantBuffers(0, 1, &pRenderDataBuffer);
		//pDevContext->HSSetConstantBuffers(1, 1, &pObjectDataBuffer);

		//pDevContext->DSSetConstantBuffers(0, 1, &pRenderDataBuffer);
		//pDevContext->DSSetConstantBuffers(1, 1, &pObjectDataBuffer);

		pDevContext->RSSetViewports(1u, &viewport);

		//pDevContext->PSSetConstantBuffers(0, 1, &pRenderDataBuffer);
		//pDevContext->PSSetConstantBuffers(1, 1, &pObjectDataBuffer);
		pDevContext->PSSetSamplers(0u, 1u, &simp);

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

		pDevContext = DX11::get().getDeviceContext();

		pRenderTarget = target;
		OKAY_ASSERT(pRenderTarget, "RenderTarget was nullptr");
		pScene = scene;
		OKAY_ASSERT(pScene, "Scene was nullptr");

		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		const DirectX::XMUINT2 dims = pRenderTarget->getDimensions();
		onTargetResize(dims.x, dims.y);

		init();
	}

	void Renderer::create(uint32_t width, uint32_t height, Ref<Scene> scene)
	{
		Ref<RenderTexture> target = createRef<RenderTexture>(width, height, RenderTexture::RENDER | RenderTexture::SHADER_READ | RenderTexture::DEPTH);
		create(target, scene);
	}

	void Renderer::shutdown()
	{
		customCamera = Entity();
		pScene = nullptr;
		pRenderTarget = nullptr;
		pDevContext = nullptr;

		DX11_RELEASE(pRenderDataBuffer);
		DX11_RELEASE(pObjectDataBuffer);
		DX11_RELEASE(simp);
		DX11_RELEASE(pPosIL);
		DX11_RELEASE(pPosUvNormIL);
		DX11_RELEASE(pMeshVS);
		DX11_RELEASE(pGrassVS);
		DX11_RELEASE(pGrassHS);
		DX11_RELEASE(pGrassDS);
		DX11_RELEASE(pNoCullRS);
		DX11_RELEASE(pDefaultPS);
		DX11_RELEASE(pGrassPS);
		viewport = D3D11_VIEWPORT();
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
		Entity camEntity = pScene->getMainCamera();
		OKAY_ASSERT(camEntity, "Invalid camera entity");

		OKAY_ASSERT(camEntity.hasComponent<Camera>(), "Camera entity doesn't have a camera component");
		const Camera& camera = camEntity.getComponent<Camera>();
		const Transform& camTransform = camEntity.getComponent<Transform>();

		using namespace DirectX;

		XMFLOAT3 upDir = XMFLOAT3(0.f, 1.f, 0.f);
		XMFLOAT3 origo = XMFLOAT3(0.f, 0.f, 0.f);
		XMMATRIX viewProj = XMMatrixLookAtLH(XMLoadFloat3(&camTransform.position), XMLoadFloat3(&origo), XMLoadFloat3(&upDir)) *
		camera.calculateProjMatrix(viewport.Width, viewport.Height);

		DirectX::XMStoreFloat4x4(&renderData.viewProjMatrix, DirectX::XMMatrixTranspose(viewProj));
		renderData.camPos = camTransform.position;
		renderData.camDir = DirectX::XMFLOAT3(0.f, 0.f, 0.f); // TODO: FIX THIS 
		
		DX11::updateBuffer(pRenderDataBuffer, &renderData, sizeof(GPURenderData));

		// Standard mesh pipeline
		pDevContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pDevContext->IASetInputLayout(pPosUvNormIL);
		pDevContext->VSSetShader(pMeshVS, nullptr, 0u);
		pDevContext->HSSetShader(nullptr, nullptr, 0u);
		pDevContext->DSSetShader(nullptr, nullptr, 0u);
		pDevContext->RSSetState(nullptr);
		pDevContext->PSSetShader(pDefaultPS, nullptr, 0u);
		pRenderTarget->clear();


		ContentBrowser& content = ContentBrowser::get();
		entt::registry& reg = pScene->getRegistry();
		auto objGroup = reg.group<Transform, MeshComponent>();
		DirectX::XMMATRIX worldMatrix{};

		for (entt::entity entity : objGroup)
		{
			auto [transform, meshComp] = objGroup.get<Transform, MeshComponent>(entity);

			const Mesh& mesh = content.getAsset<Mesh>(meshComp.meshIdx);
			const Material& material = content.getAsset<Material>(mesh.getMaterialID());
			const Material::GPUData& matGPUData = material.getGPUData();
			const Texture& diffuseTexture = content.getAsset<Texture>(material.getBaseColour());


			transform.calculateMatrix();
			worldMatrix = transform.matrix;
			DirectX::XMStoreFloat4x4(&objectData.worldMatrix, worldMatrix);
			objectData.uvOffset = matGPUData.uvOffset;
			objectData.uvTiling = matGPUData.uvTiling;
			DX11::updateBuffer(pObjectDataBuffer, &objectData, sizeof(GPUObjectData));


			pDevContext->IASetVertexBuffers(0u, Mesh::NumBuffers, mesh.getBuffers(), Mesh::Stride, Mesh::Offset);
			pDevContext->IASetIndexBuffer(mesh.getIndexBuffer(), DXGI_FORMAT_R32_UINT, 0u);

			pDevContext->PSSetShaderResources(0u, 1u, diffuseTexture.getSRV());

			pDevContext->DrawIndexed(mesh.getNumIndices(), 0u, 0);
		}

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
}
