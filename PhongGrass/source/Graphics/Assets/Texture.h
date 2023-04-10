#pragma once
#include <string>

#include "DirectX/DX11.h"

namespace Okay
{
	class Texture
	{
	public:
		Texture(const unsigned char* pData, uint32_t width, uint32_t height, std::string_view name);
		Texture(Texture&& other) noexcept;
		~Texture();
		void shutdown();

		inline void setName(std::string_view name);
		inline const std::string& getName() const;

		inline uint32_t getWidth() const;
		inline uint32_t getHeight() const;
		inline float getAspectRatio() const;

		inline ID3D11Texture2D* getTexture() const;
		inline ID3D11ShaderResourceView* getSRV() const;

	private:
		std::string name;
		const uint32_t width, height;

		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* srv;

		D3D11_TEXTURE2D_DESC createDefaultDesc() const
		{
			// Width, height, mipLevels, arraySize, format, sampleDesc, usage, bindFlags, cpuAccess, miscFlags
			return { width, height, 1,1, DXGI_FORMAT_R8G8B8A8_UNORM, {1, 0}, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0 };
		}
	};

	inline void Texture::setName(std::string_view name) 
	{
		this->name = name; 
	}
	
	inline const std::string& Texture::getName() const 
	{ 
		return name; 
	}

	inline uint32_t Texture::getWidth() const
	{
		 return width; 
	}

	inline uint32_t Texture::getHeight() const 
	{ 
		return height; 
	}

	inline float Texture::getAspectRatio() const 
	{ 
		return float(width) / float(height); 
	}

	inline ID3D11Texture2D* Texture::getTexture() const 
	{ 
		return texture; 
	}

	inline ID3D11ShaderResourceView* Texture::getSRV() const 
	{ 
		return srv; 
	}
}