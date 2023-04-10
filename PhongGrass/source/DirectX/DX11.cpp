#include "DX11.h"
#include "Application/Window.h"

#include <d3dcompiler.h>

DX11::DX11()
	:pDevice(), pDeviceContext()
{
	HRESULT hr{};
	
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	uint32_t flags = 0;
#ifndef DIST	
	flags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Device and DeviceContext
	hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
		&featureLevel, 1u, D3D11_SDK_VERSION, &pDevice, nullptr, &pDeviceContext);
	OKAY_ASSERT(SUCCEEDED(hr), "Failed initializing DirectX 11");
}

DX11::~DX11()
{
	shutdown();
}

void DX11::shutdown()
{
	DX11_RELEASE(pDeviceContext);

#if 0
	ID3D11Debug* debugger = nullptr;
	pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debugger));
	debugger->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	debugger->Release();
#endif

	DX11_RELEASE(pDevice);
}


/* ------ HELPER FUNCTIONS ------ */

bool DX11::createSwapChain(IDXGISwapChain** ppSwapChain, HWND hWnd, DXGI_USAGE dx11UsageFlags)
{
	DXGI_SWAP_CHAIN_DESC desc{};
	desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	desc.BufferCount = 1;
	desc.BufferUsage = dx11UsageFlags;

	desc.BufferDesc.Width = 0u; // 0u defaults to the window dimensions
	desc.BufferDesc.Height = 0u;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.RefreshRate.Numerator = 0u;
	desc.BufferDesc.RefreshRate.Denominator = 1u;

	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	desc.OutputWindow = hWnd;
	desc.Windowed = true;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ID3D11Device* pDevice = get().getDevice();
	IDXGIDevice* idxDevice = nullptr;
	IDXGIAdapter* adapter = nullptr;
	IDXGIFactory* factory = nullptr;

	pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&idxDevice);
	idxDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&adapter);
	adapter->GetParent(__uuidof(IDXGIFactory), (void**)&factory);

	factory->CreateSwapChain(pDevice, &desc, ppSwapChain);
	DX11_RELEASE(idxDevice);
	DX11_RELEASE(adapter);
	DX11_RELEASE(factory);

	return *ppSwapChain;
}

HRESULT DX11::createVertexBuffer(ID3D11Buffer** ppBuffer, const void* pData, UINT byteSize, bool immutable)
{
	D3D11_BUFFER_DESC desc{};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.Usage = immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = immutable ? 0 : D3D11_CPU_ACCESS_WRITE;
	desc.ByteWidth = byteSize;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA inData{};
	inData.pSysMem = pData;
	inData.SysMemPitch = inData.SysMemSlicePitch = 0;
	return get().pDevice->CreateBuffer(&desc, pData ? &inData : nullptr, ppBuffer);
}

HRESULT DX11::createIndexBuffer(ID3D11Buffer** ppBuffer, const void* pData, UINT byteSize, bool immutable)
{
	D3D11_BUFFER_DESC desc{};
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.Usage = immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = immutable ? 0 : D3D11_CPU_ACCESS_WRITE;
	desc.ByteWidth = byteSize;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA inData{};
	inData.pSysMem = pData;
	inData.SysMemPitch = inData.SysMemSlicePitch = 0;
	return get().pDevice->CreateBuffer(&desc, pData ? &inData : nullptr, ppBuffer);
}

HRESULT DX11::createConstantBuffer(ID3D11Buffer** ppBuffer, const void* pData, UINT byteSize, bool immutable)
{
	D3D11_BUFFER_DESC desc{};
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = immutable ? 0 : D3D11_CPU_ACCESS_WRITE;
	desc.ByteWidth = byteSize;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA inData{};
	inData.pSysMem = pData;
	inData.SysMemPitch = inData.SysMemSlicePitch = 0;
	return get().pDevice->CreateBuffer(&desc, pData ? &inData : nullptr, ppBuffer);
}

void DX11::updateBuffer(ID3D11Resource* pBuffer, const void* pData, UINT byteSize, ID3D11DeviceContext* pDefContext)
{
	ID3D11DeviceContext* pContext = pDefContext ? pDefContext : get().pDeviceContext;

	D3D11_MAPPED_SUBRESOURCE sub;
	if (FAILED(pContext->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub)))
		return;

	memcpy(sub.pData, pData, byteSize);
	pContext->Unmap(pBuffer, 0);
}

void DX11::updateTexture(ID3D11Texture2D* pBuffer, const void* pData, uint32_t elementByteSize, uint32_t width, uint32_t height)
{
	D3D11_BOX box{};
	box.left = 0u;
	box.top = 0u;
	box.front = 0u;
	box.back = 1u;

	if (!width || !height)
	{
		D3D11_TEXTURE2D_DESC desc{};
		pBuffer->GetDesc(&desc);
		width = width ? width : desc.Width;
		height = height ? height : desc.Height;
	}
	box.right = width;
	box.bottom = height;

	get().pDeviceContext->UpdateSubresource(pBuffer, 0, &box, pData, width * elementByteSize, 0u);
}

HRESULT DX11::createStructuredBuffer(ID3D11Buffer** ppBuffer, const void* pData, UINT eleByteSize, UINT numElements, bool immutable)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = eleByteSize * numElements;
	desc.CPUAccessFlags = immutable ? 0 : D3D11_CPU_ACCESS_WRITE;
	desc.Usage = immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = eleByteSize;
	D3D11_SUBRESOURCE_DATA inData;
	inData.pSysMem = pData;
	inData.SysMemPitch = 0;
	inData.SysMemSlicePitch = 0;

	return get().pDevice->CreateBuffer(&desc, pData ? &inData : nullptr, ppBuffer);
}

HRESULT DX11::createStructuredSRV(ID3D11ShaderResourceView** ppSRV, ID3D11Buffer* pBuffer, UINT numElements)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Buffer.FirstElement = 0;
	desc.Buffer.NumElements = numElements;

	return get().pDevice->CreateShaderResourceView(pBuffer, &desc, ppSRV);
}



class IncludeReader : public ID3DInclude
{
public:

	// Inherited via ID3DInclude
	virtual HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override
	{
		std::ifstream reader(std::string(SHADER_PATH) + pFileName);
		if (!reader)
			return E_FAIL;

		reader.seekg(0, std::ios::end);
		includeBuffer.reserve((size_t)reader.tellg());
		reader.seekg(0, std::ios::beg);

		includeBuffer.assign(std::istreambuf_iterator<char>(reader), std::istreambuf_iterator<char>());

		*ppData = includeBuffer.c_str();
		*pBytes = (uint32_t)includeBuffer.size();

		return S_OK;
	}

	virtual HRESULT __stdcall Close(LPCVOID pData) override
	{
		return S_OK;
	}

private:
	std::string includeBuffer;
};

template bool DX11::createShader(std::string_view path, ID3D11VertexShader** ppShader, std::string* pOutShaderData);
template bool DX11::createShader(std::string_view path, ID3D11HullShader** ppShader, std::string* pOutShaderData);
template bool DX11::createShader(std::string_view path, ID3D11DomainShader** ppShader, std::string* pOutShaderData);
template bool DX11::createShader(std::string_view path, ID3D11PixelShader** ppShader, std::string* pOutShaderData);

template<typename ShaderType>
bool DX11::createShader(std::string_view path, ShaderType** ppShader, std::string* pOutShaderData)
{
	if (!ppShader)
		return false;

	std::string_view fileEnding = Okay::getFileEnding(path);

	if (fileEnding == ".cso" || fileEnding == ".CSO")
	{
		std::string shaderData;

		// if pOutShaderData is nullptr, it is simply used to point to the actual buffer
		// Allowing faster and (imo) a bit cleaner code 
		if (!pOutShaderData)
			pOutShaderData = &shaderData;

		if (!Okay::readBinary(path, *pOutShaderData))
			return false;

		if constexpr (std::is_same<ShaderType, ID3D11VertexShader>())
			return SUCCEEDED(DX11::get().getDevice()->CreateVertexShader(pOutShaderData->c_str(), pOutShaderData->length(), nullptr, ppShader));

		else if constexpr (std::is_same<ShaderType, ID3D11PixelShader>())
			return SUCCEEDED(DX11::get().getDevice()->CreatePixelShader(pOutShaderData->c_str(), pOutShaderData->length(), nullptr, ppShader));
	}
	else
	{
		// Convert char-string to wchar_t-string
		wchar_t* lpPath = new wchar_t[path.size() + 1ull]{};
		mbstowcs_s(nullptr, lpPath, path.size() + 1ull, path.data(), path.size());

		ID3DBlob* shaderData = nullptr;
		ID3DBlob* compileErrors = nullptr;

		// If neither are defined a compiler error is produced. Forcing the user to ensure the correct one is used
#if defined(DIST)
		uint32_t optimizationLevel = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#elif defined(_DEBUG)
		uint32_t optimizationLevel = D3DCOMPILE_OPTIMIZATION_LEVEL0;
#elif defined(NDEBUG)
		uint32_t optimizationLevel = D3DCOMPILE_OPTIMIZATION_LEVEL2;
#endif

		const char* shaderTypeTarget = nullptr;
		if		constexpr (std::is_same<ShaderType, ID3D11VertexShader>())	shaderTypeTarget = "vs_5_0";
		else if constexpr (std::is_same<ShaderType, ID3D11PixelShader>())	shaderTypeTarget = "ps_5_0";

		IncludeReader includer;
		HRESULT hr = D3DCompileFromFile(lpPath, nullptr, &includer, "main", shaderTypeTarget, optimizationLevel, 0u, &shaderData, &compileErrors);
		OKAY_DELETE_ARRAY(lpPath);

		if (FAILED(hr))
		{
			printf("Shader compilation error: %s\n", compileErrors ? (char*)compileErrors->GetBufferPointer() : "No information, file might not have been found");
			return false;
		}

		if (pOutShaderData)
			pOutShaderData->assign((char*)shaderData->GetBufferPointer(), shaderData->GetBufferSize());

		if constexpr (std::is_same<ShaderType, ID3D11VertexShader>())
			return SUCCEEDED(DX11::get().getDevice()->CreateVertexShader(shaderData->GetBufferPointer(), shaderData->GetBufferSize(), nullptr, (ID3D11VertexShader**)ppShader));

		else if constexpr (std::is_same<ShaderType, ID3D11PixelShader>())
			return SUCCEEDED(DX11::get().getDevice()->CreatePixelShader(shaderData->GetBufferPointer(), shaderData->GetBufferSize(), nullptr, (ID3D11PixelShader**)ppShader));
	}

	return false;
}