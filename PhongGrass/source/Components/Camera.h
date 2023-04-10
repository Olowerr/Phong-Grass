#pragma once

#include <DirectXMath.h>

namespace Okay
{
	struct Camera
	{
		// TODO: Add support for target point camera
		// without forcing the calculation on the user

		float fov = DirectX::XM_PIDIV2;
		float nearZ = 0.1f;
		float farZ = 1000.f;

		Camera() = default;

		Camera(float fov, float nearZ, float farZ)
			:fov(fov), nearZ(nearZ), farZ(farZ)
		{
		}

		inline DirectX::XMMATRIX calculateProjMatrix(float width, float height) const
		{
			return DirectX::XMMatrixPerspectiveFovLH(fov, width / height, nearZ, farZ);
		}
	};
}