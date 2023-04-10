#pragma once

#include "Okay/Okay.h"
#include "DirectX/DX11.h"

#include <memory>

namespace Okay
{
	class SkyBox
	{
	public:
		SkyBox();
		~SkyBox();
		void shutdown();

		bool create(std::string_view path);
		inline ID3D11ShaderResourceView* const* getTextureCubeSRV() const;

		inline const std::string& getTextureName() const;

	private:
		std::string textureName;

		ID3D11ShaderResourceView* pTextureCubeSRV;
	};

	inline ID3D11ShaderResourceView* const* SkyBox::getTextureCubeSRV() const { return &pTextureCubeSRV; }
	inline const std::string& SkyBox::getTextureName() const { return textureName; }
}