#pragma once

#include <DirectXMath.h>

namespace Okay
{
	struct DirectionalLight
	{
		DirectionalLight() = default;
		DirectionalLight(float intensity, const DirectX::XMFLOAT3& colour)
			:intensity(intensity), colour(colour)
		{ }

		DirectX::XMFLOAT3 colour = DirectX::XMFLOAT3(1.f, 1.f, 1.f);
		float intensity = 1.f;
	};
}