#include "SkyBox.h"

#include "ContentBrowser.h"
#include "Graphics/Assets/ImpExp/stb_image.h"

namespace Okay
{
	SkyBox::SkyBox()
		:pTextureCubeSRV(nullptr)
	{
	}

	SkyBox::~SkyBox()
	{
		shutdown();
	}

	void SkyBox::shutdown()
	{
		DX11_RELEASE(pTextureCubeSRV);
	}

	bool SkyBox::create(std::string_view path)
	{
		int imgWidth = 0, imgHeight = 0;

		// Using a 4 byte type ptr so one "step" is one pixel instead of one colour value
		uint32_t* pImgData = (uint32_t*)stbi_load(path.data(), &imgWidth, &imgHeight, nullptr, 4);
		if (!pImgData)
			return false;

		if (imgWidth % 4 != 0 || imgHeight % 3 != 0)
		{
			stbi_image_free(pImgData);
			return false;
		}

		shutdown();

		size_t pos = path.find_last_of('/');
		pos = pos == std::string_view::npos ? path.find_last_of('\\') : pos;
		textureName = pos == std::string_view::npos ? path : path.substr(pos + 1ull);

		const uint32_t width = imgWidth / 4;
		const uint32_t height = imgHeight / 3;
		const uint32_t byteWidth = width * height * 4u;

		D3D11_SUBRESOURCE_DATA data[6]{};
		for (size_t i = 0; i < 6; i++)
		{
			data[i].pSysMem = malloc(byteWidth);
			data[i].SysMemPitch = uint32_t(width * 4u);
			data[i].SysMemSlicePitch = 0u;
		}
	
		// The coursor points to the location of each side
		uint32_t* coursor = nullptr;

		auto copyImgSection = [&](uint32_t* pTarget)
		{
			for (uint32_t i = 0; i < height; i++)
			{
				memcpy(pTarget, coursor, width * 4ull);
				pTarget += width;
				coursor += imgWidth;
			}
		};

		// Positive X
		coursor = pImgData + imgWidth * height + width * 2u;
		copyImgSection((uint32_t*)data[0].pSysMem);

		// Negative X
		coursor = pImgData + imgWidth * height;
		copyImgSection((uint32_t*)data[1].pSysMem);

		// Positive Y
		coursor = pImgData + width;
		copyImgSection((uint32_t*)data[2].pSysMem);

		// Negative Y
		coursor = pImgData + imgWidth * height * 2u + width;
		copyImgSection((uint32_t*)data[3].pSysMem);

		// Positive Z
		coursor = pImgData + imgWidth * height + width;
		copyImgSection((uint32_t*)data[4].pSysMem);
		
		// Negative Z
		coursor = pImgData + imgWidth * height + width * 3u;
		copyImgSection((uint32_t*)data[5].pSysMem);

		stbi_image_free(pImgData);

		D3D11_TEXTURE2D_DESC desc{};
		desc.ArraySize = 6u;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0u;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_IMMUTABLE;

		ID3D11Texture2D* pTextureCube = nullptr;
		HRESULT hr = DX11::get().getDevice()->CreateTexture2D(&desc, data, &pTextureCube);

		for (size_t i = 0; i < 6; i++)
			free((void*)data[i].pSysMem);

		OKAY_ASSERT(SUCCEEDED(hr), "Failed creating Texture Cube");

		hr = DX11::get().getDevice()->CreateShaderResourceView(pTextureCube, nullptr, &pTextureCubeSRV);
		pTextureCube->Release();

		OKAY_ASSERT(SUCCEEDED(hr), "Failed creating Texture Cube SRV");

		return true;
	}
}
