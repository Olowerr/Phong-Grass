#include "RenderTexture.h"
#include "DirectX/DX11.h"

namespace Okay
{
	RenderTexture::RenderTexture()
		:buffer(nullptr), rtv(nullptr), srv(nullptr), uav(nullptr),
		depthBuffer(nullptr), dsv(nullptr), flags(0u), format(Format::INVALID), dims(0u, 0u)
	{

	}

	RenderTexture::RenderTexture(ID3D11Texture2D* texture, uint32_t flags)
		:buffer(nullptr), rtv(nullptr), srv(nullptr), uav(nullptr),
		depthBuffer(nullptr), dsv(nullptr)
	{
		create(texture, flags);
	}

	RenderTexture::RenderTexture(uint32_t width, uint32_t height, uint32_t flags, Format format)
		:buffer(nullptr), rtv(nullptr), srv(nullptr), uav(nullptr),
		depthBuffer(nullptr), dsv(nullptr)
	{
		create(width, height, flags, format);
	}

	RenderTexture::~RenderTexture()
	{
		shutdown();
	}

	void RenderTexture::shutdown()
	{
		DX11_RELEASE(buffer);
		DX11_RELEASE(rtv);
		DX11_RELEASE(srv);
		DX11_RELEASE(uav);

		DX11_RELEASE(depthBuffer);
		DX11_RELEASE(dsv);

		dims = DirectX::XMUINT2();
		flags = 0u;
		format = Format::INVALID;
	}
	
	void RenderTexture::create(ID3D11Texture2D* texture, uint32_t flags)
	{
		OKAY_ASSERT(texture, "Texture was nullptr");

		shutdown();
		buffer = texture; 
		buffer->AddRef();

		D3D11_TEXTURE2D_DESC desc{};
		buffer->GetDesc(&desc);

		// TODO: Add format safety
		switch (desc.Format)
		{
		default:
			break;
		case DXGI_FORMAT_R8_UNORM:
			format = Format::F_8X1;
			break;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			format = Format::F_8X4;
			break;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			format = Format::F_32X4;
			break;
		}

		dims.x = desc.Width;
		dims.y = desc.Height;

		readFlgs(flags);
	}

	void RenderTexture::create(uint32_t width, uint32_t height, uint32_t flags, Format format)
	{
		shutdown();

		ID3D11Device* pDevice = DX11::get().getDevice();

		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = width;
		desc.Height = height;
		desc.ArraySize = 1u;
		desc.CPUAccessFlags = 0u;
		desc.MipLevels = 1u;
		desc.MiscFlags = 0u;
		desc.SampleDesc.Count = 1u;
		desc.SampleDesc.Quality = 0u;
		desc.Usage = D3D11_USAGE_DEFAULT;

		this->format = format;
		if		(format == Format::F_8X1)  desc.Format = DXGI_FORMAT_R8_UNORM;
		else if (format == Format::F_8X4)  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		else if (format == Format::F_32X4) desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

		desc.BindFlags =
			(CHECK_BIT(flags, BitPos::B_RENDER) ? D3D11_BIND_RENDER_TARGET : 0u) |
			(CHECK_BIT(flags, BitPos::B_SHADER_READ) ? D3D11_BIND_SHADER_RESOURCE : 0u) |
			(CHECK_BIT(flags, BitPos::B_SHADER_WRITE) ? D3D11_BIND_UNORDERED_ACCESS : 0u);

		pDevice->CreateTexture2D(&desc, nullptr, &buffer);
		OKAY_ASSERT(buffer, "Failed creating RenderTexture");

		dims.x = width;
		dims.y = height;

		readFlgs(flags);
	}

	void RenderTexture::clear()
	{
		static float white[4]{ 1.f, 1.f, 1.f, 1.f };
		DX11::get().getDeviceContext()->ClearRenderTargetView(rtv, white);

		if (dsv)
			DX11::get().getDeviceContext()->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.f, 0);
	}

	void RenderTexture::resize(uint32_t width, uint32_t height)
	{
		// The "right way" to get the reference count requires some additional stuff (seemed annoying)
		buffer->AddRef();
		if (buffer->Release() != 1u) // Don't resize if not owning (the other references won't update)
			return;

		create(width, height, flags, format);

		for (auto& callback : callbacks)
			callback(width, height);
	}

	void RenderTexture::readFlgs(uint32_t flags)
	{
		this->flags = flags;
		ID3D11Device* pDevice = DX11::get().getDevice();

		if (CHECK_BIT(flags, BitPos::B_RENDER))
		{
			pDevice->CreateRenderTargetView(buffer, nullptr, &rtv);
			OKAY_ASSERT(rtv, "Failed creating RTV");
		}
		if (CHECK_BIT(flags, BitPos::B_SHADER_READ))
		{
			pDevice->CreateShaderResourceView(buffer, nullptr, &srv);
			OKAY_ASSERT(srv, "Failed creating SRV");
		}
		if (CHECK_BIT(flags, BitPos::B_SHADER_WRITE))
		{
			pDevice->CreateUnorderedAccessView(buffer, nullptr, &uav);
			OKAY_ASSERT(uav, "Failed creating UAV");
		}
		if (CHECK_BIT(flags, BitPos::B_DEPTH))
		{
			D3D11_TEXTURE2D_DESC desc;
			buffer->GetDesc(&desc);

			desc.Format = DXGI_FORMAT_D32_FLOAT;
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

			pDevice->CreateTexture2D(&desc, nullptr, &depthBuffer);
			OKAY_ASSERT(buffer, "Failed creating DepthBuffer");

			pDevice->CreateDepthStencilView(depthBuffer, nullptr, &dsv);
			OKAY_ASSERT(dsv, "Failed creating DSV");
		}

	}
}
