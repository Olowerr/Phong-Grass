#pragma once
#include <DirectXMath.h>

namespace Okay
{
	struct Transform
	{
		Transform() = default;
		DirectX::XMFLOAT3 position	= DirectX::XMFLOAT3(0.f, 0.f, 0.f);
		DirectX::XMFLOAT3 rotation	= DirectX::XMFLOAT3(0.f, 0.f, 0.f);
		DirectX::XMFLOAT3 scale		= DirectX::XMFLOAT3(1.f, 1.f, 1.f);

		DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();

		// TODO: Implement these
		//glm::vec3 forward() const { return glm::normalize(glm::vec3(matrix[2])); }
		//glm::vec3 up() const	  { return glm::normalize(glm::vec3(matrix[1])); }
		//glm::vec3 right() const   { return glm::normalize(glm::vec3(matrix[0])); }

		void calculateMatrix()
		{
			matrix =
				DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) *
				DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) *
				DirectX::XMMatrixTranslation(position.x, position.y, position.z);
		}
	};
}