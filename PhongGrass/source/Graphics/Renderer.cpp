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

		{
			ContentBrowser& content = ContentBrowser::get();
			if (skyBoxMeshId = content.getAssetID<Mesh>("cube"); skyBoxMeshId == INVALID_UINT)
			{
				result = content.importFile(RESOURCES_PATH "meshes/cube.fbx");
				OKAY_ASSERT(result, "Failed loading skyBox mesh");
				skyBoxMeshId = content.getAmount<Mesh>() - 1u;
			}
		}

		// Buffers
		{
			hr = DX11::createConstantBuffer(&pRenderDataBuffer, nullptr, sizeof(GPURenderData), false);
			OKAY_ASSERT(SUCCEEDED(hr), "Failed creating pRenderDataBuffer");

			hr = DX11::createConstantBuffer(&pObjectDataBuffer, nullptr, sizeof(GPUObjectData), false);
			OKAY_ASSERT(SUCCEEDED(hr), "Failed creating pObjectDataBuffer");

			hr = DX11::createConstantBuffer(&pGrassTessDataBuffer, nullptr, sizeof(GPUGrassData), false);
			OKAY_ASSERT(SUCCEEDED(hr), "Failed creating pGrassTessDataBuffer");
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
			
			rsDesc.FillMode = D3D11_FILL_WIREFRAME;
			hr = pDevice->CreateRasterizerState(&rsDesc, &pNoCullWireframeRS);
			OKAY_ASSERT(SUCCEEDED(hr), "Failed creating noCullWireframeRS");
			

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
				{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"UV",			0, DXGI_FORMAT_R32G32_FLOAT,	2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
			};

			std::string shaderData;

			{
				result = DX11::createShader(SHADER_PATH "MeshVS.hlsl", &pMeshVS, &shaderData);
				OKAY_ASSERT(result, "Failed creating vertex shader");

				hr = pDevice->CreateInputLayout(inputLayoutDesc, 3u, shaderData.c_str(), shaderData.length(), &pPosNormUvIL);
				OKAY_ASSERT(SUCCEEDED(hr), "Failed creating input layout");
			}

			{
				result = DX11::createShader(SHADER_PATH "InstancedTessVS.hlsl", &pInstancedTessVS, &shaderData);
				OKAY_ASSERT(result, "Failed creating instanced vertex shader");
				
				hr = pDevice->CreateInputLayout(inputLayoutDesc, 2u, shaderData.c_str(), shaderData.length(), &pPosNormIL);
				OKAY_ASSERT(SUCCEEDED(hr), "Failed creating input layout");

				result = DX11::createShader(SHADER_PATH "InstancedStaticVS.hlsl", &pInstancedStaticVS);
				OKAY_ASSERT(result, "Failed creating instanced vertex shader");
			}

			{
				result = DX11::createShader(SHADER_PATH "SkyBoxVS.hlsl", &pSkyBoxVS, &shaderData);
				OKAY_ASSERT(result, "Failed creating skybox vertex shader");
				
				hr = pDevice->CreateInputLayout(inputLayoutDesc, 1u, shaderData.c_str(), shaderData.length(), &pPosIL);
				OKAY_ASSERT(SUCCEEDED(hr), "Failed creating skybox input layout");
				
				result = DX11::createShader(SHADER_PATH "SkyBoxPS.hlsl", &pSkyBoxPS);
				OKAY_ASSERT(result, "Failed creating skybox pixel shader");
			}

			{
				D3D_SHADER_MACRO defines[] =
				{
					{"MODE", ""},
					{NULL, NULL},
				};
				const char* modeValues[3] = { "0", "1", "2" };

				for (size_t i = 0; i < 3; i++)
				{
					defines[0].Definition = modeValues[i];
					result = DX11::createShader(SHADER_PATH "GrassHS.hlsl", &pGrassHS[i], nullptr, defines);
					OKAY_ASSERT(result, "Failed creating grass hull shhader");
				}

				pGrassUsedHS = pGrassHS[0];
				pGrassUsedHS->AddRef();
			}

			result = DX11::createShader(SHADER_PATH "GrassDS.hlsl", &pGrassDS);
			OKAY_ASSERT(result, "Failed creating grass domain shhader");

			result = DX11::createShader(SHADER_PATH "GrassPS.hlsl", &pGrassPS);
			OKAY_ASSERT(result, "Failed creating grass pixel shader");

			result = DX11::createShader(SHADER_PATH "DefaultPS.hlsl", &pDefaultPS);
			OKAY_ASSERT(result, "Failed creating Default pixel shader");

		}

		// Depthstenil states
		{
			D3D11_DEPTH_STENCIL_DESC dsDesc{};
			dsDesc.DepthEnable = true;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			dsDesc.StencilEnable = false;
			dsDesc.StencilReadMask = 0;
			dsDesc.StencilWriteMask = 0;

			hr = pDevice->CreateDepthStencilState(&dsDesc, &pLessEqualDSS);
			OKAY_ASSERT(SUCCEEDED(hr), "Failed creating lessEqual Depth Stencil State");
		}

		pDevContext->VSSetConstantBuffers(0, 1, &pRenderDataBuffer);
		pDevContext->VSSetConstantBuffers(1, 1, &pObjectDataBuffer);
		pDevContext->VSSetConstantBuffers(2, 1, &pGrassTessDataBuffer);
		pDevContext->VSSetSamplers(0u, 1u, &simp);
		
		pDevContext->HSSetConstantBuffers(0, 1, &pRenderDataBuffer);
		pDevContext->HSSetConstantBuffers(1, 1, &pObjectDataBuffer);
		pDevContext->HSSetConstantBuffers(2, 1, &pGrassTessDataBuffer);

		pDevContext->DSSetConstantBuffers(0, 1, &pRenderDataBuffer);
		pDevContext->DSSetConstantBuffers(1, 1, &pObjectDataBuffer);
		pDevContext->DSSetConstantBuffers(2, 1, &pGrassTessDataBuffer);

		pDevContext->RSSetViewports(1u, &viewport);

		pDevContext->PSSetConstantBuffers(0, 1, &pRenderDataBuffer);
		pDevContext->PSSetConstantBuffers(1, 1, &pObjectDataBuffer);
		pDevContext->PSSetConstantBuffers(2, 1, &pGrassTessDataBuffer);
		pDevContext->PSSetSamplers(0u, 1u, &simp);

		pDevContext->OMSetRenderTargets(1u, pRenderTarget->getRTV(), *pRenderTarget->getDSV());
	}

	void Renderer::initGrass(const std::vector<DirectX::XMFLOAT4X4>& grassTransforms)
	{
		DX11_RELEASE(pGrassTransformSRV);

		numGrassBlades = (uint32_t)grassTransforms.size();

		HRESULT hr;
		ID3D11Buffer* pGrassTransformBuffer = nullptr;

		hr = DX11::createStructuredBuffer(&pGrassTransformBuffer, grassTransforms.data(), (uint32_t)sizeof(DirectX::XMFLOAT4X4), numGrassBlades);
		OKAY_ASSERT(SUCCEEDED(hr), "Failed creating grass buffer");

		hr = DX11::createStructuredSRV(&pGrassTransformSRV, pGrassTransformBuffer, numGrassBlades);
		OKAY_ASSERT(SUCCEEDED(hr), "Failed creating grass SRV");

		pGrassTransformBuffer->Release();

		pDevContext->VSSetShaderResources(2, 1, &pGrassTransformSRV);
		pDevContext->HSSetShaderResources(2, 1, &pGrassTransformSRV);
	}

	Renderer::Renderer()
		:pScene(nullptr), pRenderTarget(nullptr), pDevContext(nullptr), viewport(), renderData(), objectData()
	{
	}

	Renderer::Renderer(Ref<RenderTexture> target, Ref<Scene> scene)
		:pScene(nullptr), pRenderTarget(nullptr), pDevContext(nullptr), viewport(), renderData(), objectData()
	{
		create(target, scene);
	}

	Renderer::Renderer(uint32_t width, uint32_t height, Ref<Scene> scene)
		:pScene(nullptr), pRenderTarget(nullptr), pDevContext(nullptr), viewport(), renderData(), objectData()
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
		DX11_RELEASE(pGrassTessDataBuffer);
		DX11_RELEASE(pGrassTransformSRV);
		DX11_RELEASE(simp);
		DX11_RELEASE(pPosNormUvIL);
		DX11_RELEASE(pPosNormIL);
		DX11_RELEASE(pPosIL);
		DX11_RELEASE(pMeshVS);
		DX11_RELEASE(pInstancedTessVS);
		DX11_RELEASE(pInstancedStaticVS);
		DX11_RELEASE(pSkyBoxVS);
		DX11_RELEASE(pGrassHS[0]);
		DX11_RELEASE(pGrassHS[1]);
		DX11_RELEASE(pGrassHS[2]);
		DX11_RELEASE(pGrassUsedHS);
		DX11_RELEASE(pGrassDS);
		DX11_RELEASE(pNoCullRS);
		DX11_RELEASE(pNoCullWireframeRS);
		DX11_RELEASE(pDefaultPS);
		DX11_RELEASE(pGrassPS);
		DX11_RELEASE(pSkyBoxPS);
		DX11_RELEASE(pLessEqualDSS);
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

	template<typename ShaderType>
	void imGuiUpdateShader(ShaderType** ppShader, std::string_view shaderPath, std::string_view shaderName)
	{
		if (!ImGui::Button(shaderName.data()))
			return;

		ShaderType* newShader = nullptr;
		if (!DX11::createShader(shaderPath, &newShader))
			return;

		DX11_RELEASE((*ppShader));
		*ppShader = newShader;
	}

	void Renderer::imGui()
	{
		if (!ImGui::Begin("Renderer"))
		{
			ImGui::End();
			return;
		}

		ImGui::Text("Standard shaders");
		imGuiUpdateShader(&pMeshVS, SHADER_PATH "MeshVS.hlsl", "MeshVS");
		imGuiUpdateShader(&pSkyBoxVS, SHADER_PATH "SkyBoxVS.hlsl", "SkyBoxVS");
		imGuiUpdateShader(&pDefaultPS, SHADER_PATH "DefaultPS.hlsl", "DefaultPS");
		imGuiUpdateShader(&pSkyBoxPS, SHADER_PATH "SkyBoxPS.hlsl", "SkyBoxPS");

		ImGui::Separator();
		ImGui::Text("Grass shaders");
		imGuiUpdateShader(&pInstancedTessVS, SHADER_PATH "InstancedTessVS.hlsl", "InstancedTessVS");
		imGuiUpdateShader(&pInstancedStaticVS, SHADER_PATH "InstancedStaticVS.hlsl", "InstancedStaticVS");
		{
			bool allGood = ImGui::Button("GrassHS");

			ID3D11HullShader* newShaders[3]{};
			D3D_SHADER_MACRO defines[] =
			{
				{"MODE", ""},
				{NULL, NULL},
			};
			const char* modeValues[3] = { "0", "1", "2" };

			for (size_t i = 0; i < 3 && allGood; i++)
			{
				defines[0].Definition = modeValues[i];
				bool result = DX11::createShader(SHADER_PATH "GrassHS.hlsl", &newShaders[i], nullptr, defines);
				if (!result)
					allGood = false;
			}

			if (!allGood)
			{
				for (uint32_t i = 0; i < 3; i++)
					DX11_RELEASE(newShaders[i]);
			}
			else
			{
				for (uint32_t i = 0; i < 3; i++)
				{
					DX11_RELEASE(pGrassHS[i]);
					pGrassHS[i] = newShaders[i];
				}
				DX11_RELEASE(pGrassUsedHS);
				pGrassUsedHS = pGrassHS[0];
				pGrassUsedHS->AddRef();
			}
		}
		imGuiUpdateShader(&pGrassDS, SHADER_PATH "GrassDS.hlsl", "GrassDS");
		imGuiUpdateShader(&pGrassPS, SHADER_PATH "GrassPS.hlsl", "GrassPS");

		

		ImGui::Separator();
		ImGui::Checkbox("Wireframe grass", &wireFrameGrass);

		ImGui::End();
	}

	void Renderer::prepareRender()
	{
		Entity camEntity = pScene->getMainCamera();
		OKAY_ASSERT(camEntity, "Invalid camera entity");

		OKAY_ASSERT(camEntity.hasComponent<Camera>(), "Camera entity doesn't have a camera component");
		const Camera& camera = camEntity.getComponent<Camera>();
		Transform& camTransform = camEntity.getComponent<Transform>();
		camTransform.calculateMatrix();

		using namespace DirectX;

		XMMATRIX viewProj = XMMatrixLookToLH(XMLoadFloat3(&camTransform.position), camTransform.forwardXM(), camTransform.upXM()) *
			camera.calculateProjMatrix(viewport.Width, viewport.Height);

		DirectX::XMStoreFloat4x4(&renderData.viewProjMatrix, DirectX::XMMatrixTranspose(viewProj));
		renderData.camPos = camTransform.position;
		renderData.camDir = camTransform.forward();

		DX11::updateBuffer(pRenderDataBuffer, &renderData, sizeof(GPURenderData));

		pRenderTarget->clear();

		const DirectX::XMUINT2 dims = pRenderTarget->getDimensions();
		viewport.Width = (float)dims.x;
		viewport.Height = (float)dims.y;

		pDevContext->RSSetViewports(1u, &viewport);
		pDevContext->OMSetRenderTargets(1, pRenderTarget->getRTV(), *pRenderTarget->getDSV());
	}

	void Renderer::renderObjects()
	{
		using namespace DirectX;

		pDevContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pDevContext->IASetInputLayout(pPosNormUvIL);
		pDevContext->VSSetShader(pMeshVS, nullptr, 0u);
		pDevContext->HSSetShader(nullptr, nullptr, 0u);
		pDevContext->DSSetShader(nullptr, nullptr, 0u);
		pDevContext->RSSetState(nullptr);
		pDevContext->PSSetShader(pDefaultPS, nullptr, 0u);

		ContentBrowser& content = ContentBrowser::get();
		entt::registry& reg = pScene->getRegistry();
		auto objGroup = reg.group<Transform, MeshComponent>();
		XMMATRIX worldMatrix{};

		ID3D11ShaderResourceView* textures[2]{};

		for (entt::entity entity : objGroup)
		{
			auto [transform, meshComp] = objGroup.get<Transform, MeshComponent>(entity);

			const Mesh& mesh = content.getAsset<Mesh>(meshComp.meshIdx);
			const Material& material = content.getAsset<Material>(meshComp.materialIdx);
			const Material::GPUData& matGPUData = material.getGPUData();
			const Texture& diffuseTexture = content.getAsset<Texture>(material.getBaseColour());

			textures[0] = *content.getAsset<Texture>(material.getBaseColour()).getSRV();
			textures[1] = *content.getAsset<Texture>(material.getSpecular()).getSRV();

			transform.calculateMatrix();
			worldMatrix = transform.matrix;
			XMStoreFloat4x4(&objectData.worldMatrix, XMMatrixTranspose(worldMatrix));
			objectData.uvOffset = matGPUData.uvOffset;
			objectData.uvTiling = matGPUData.uvTiling;
			objectData.shinyness = matGPUData.shinynessExp;
			DX11::updateBuffer(pObjectDataBuffer, &objectData, sizeof(GPUObjectData));


			pDevContext->IASetVertexBuffers(0u, Mesh::NumBuffers, mesh.getBuffers(), Mesh::Stride, Mesh::Offset);
			pDevContext->IASetIndexBuffer(mesh.getIndexBuffer(), DXGI_FORMAT_R32_UINT, 0u);

			pDevContext->PSSetShaderResources(0u, 2u, textures);

			pDevContext->DrawIndexed(mesh.getNumIndices(), 0u, 0);
		}
	}

	void Renderer::bindGrassLayout()
	{
		const Mesh& grassMesh = ContentBrowser::get().getAsset<Mesh>(grassMeshId);

		pDevContext->IASetVertexBuffers(0u, 2u, grassMesh.getBuffers(), Mesh::Stride, Mesh::Offset);
		pDevContext->IASetInputLayout(pPosNormIL);
		pDevContext->IASetIndexBuffer(grassMesh.getIndexBuffer(), DXGI_FORMAT_R32_UINT, 0u);
		pDevContext->RSSetState(wireFrameGrass ? pNoCullWireframeRS : pNoCullRS);
		pDevContext->PSSetShader(pGrassPS, nullptr, 0u);
	}

	void Renderer::renderStaticGrass()
	{
		bindGrassLayout();
		pDevContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pDevContext->VSSetShader(pInstancedStaticVS, nullptr, 0u);
		pDevContext->HSSetShader(nullptr, nullptr, 0u);
		pDevContext->DSSetShader(nullptr, nullptr, 0u);

		const Mesh& grassMesh = ContentBrowser::get().getAsset<Mesh>(grassMeshId);
		pDevContext->DrawIndexedInstanced(grassMesh.getNumIndices(), numGrassBlades, 0u, 0, 0u);
	}

	void Renderer::renderPhongGrass()
	{
		bindGrassLayout();
		pDevContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
		pDevContext->VSSetShader(pInstancedTessVS, nullptr, 0u);
		pDevContext->HSSetShader(pGrassUsedHS, nullptr, 0u);
		pDevContext->DSSetShader(pGrassDS, nullptr, 0u);

		const Mesh& grassMesh = ContentBrowser::get().getAsset<Mesh>(grassMeshId);
		pDevContext->DrawIndexedInstanced(grassMesh.getNumIndices(), numGrassBlades, 0u, 0, 0u);
	}

	void Renderer::renderSkyBox()
	{
		Mesh& skyBoxMesh = ContentBrowser::get().getAsset<Mesh>(skyBoxMeshId);

		pDevContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pDevContext->IASetInputLayout(pPosIL);
		pDevContext->IASetVertexBuffers(0u, 1u, skyBoxMesh.getBuffers(), Mesh::Stride, Mesh::Offset);
		pDevContext->IASetIndexBuffer(skyBoxMesh.getIndexBuffer(), DXGI_FORMAT_R32_UINT, 0u);
		pDevContext->VSSetShader(pSkyBoxVS, nullptr, 0u);
		pDevContext->HSSetShader(nullptr, nullptr, 0u);
		pDevContext->DSSetShader(nullptr, nullptr, 0u);
		pDevContext->RSSetState(pNoCullRS);
		pDevContext->PSSetShader(pSkyBoxPS, nullptr, 0u);
		pDevContext->OMSetDepthStencilState(pLessEqualDSS, 0u);

		pDevContext->DrawIndexed(skyBoxMesh.getNumIndices(), 0u, 0);
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
