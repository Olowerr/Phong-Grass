#include "Texture.h"

namespace Okay
{
	Texture::Texture(const unsigned char* pData, uint32_t width, uint32_t height, std::string_view name)
		:width(width), height(height)
	{
		const size_t pos = name.find_last_of('.');
		this->name.assign(name.data(), pos == std::string_view::npos ? name.size() : pos);

		D3D11_TEXTURE2D_DESC desc = createDefaultDesc();
		D3D11_SUBRESOURCE_DATA initData{pData, width * 4, 0};

		DX11& dx11 = DX11::get();
		dx11.getDevice()->CreateTexture2D(&desc, &initData, &texture);
		OKAY_ASSERT(texture != nullptr, "Failed creating DX11 Texture");

		dx11.getDevice()->CreateShaderResourceView(texture, nullptr, &srv);
		OKAY_ASSERT(srv != nullptr, "Failed creating DX11 SRV");
	}

	Texture::Texture(Texture&& other) noexcept
		:width(other.width), height(other.height), name(std::move(other.name))
	{
		texture = other.texture;
		other.texture = nullptr;

		srv = other.srv;
		other.srv = nullptr;

		const_cast<uint32_t&>(other.width) = 0u;
		const_cast<uint32_t&>(other.height) = 0u;
	}

	Texture::~Texture()
	{
		shutdown();
	}

	void Texture::shutdown()
	{
		DX11_RELEASE(texture);
		DX11_RELEASE(srv);
	}
}
