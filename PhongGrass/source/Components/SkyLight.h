#pragma once

#include "Okay/Okay.h"
#include "Graphics/SkyBox.h"

#include <DirectXMath.h>

#include <memory>

namespace Okay
{
	struct SkyLight
	{
		SkyLight() = default;

		// TODO: Change to shared_ptr ? 
		std::unique_ptr<SkyBox> skyBox = std::make_unique<SkyBox>();

		DirectX::XMFLOAT3 ambientTint = DirectX::XMFLOAT3(1.f, 1.f, 1.f);
		float ambientIntensity = 0.2f;

		DirectX::XMFLOAT3 sunColour = DirectX::XMFLOAT3(1.f, 1.f, 1.f);
		float sunShrink = 50.f; // Subject to change
		float sunIntensity = 1.f;
	};
}