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

		DirectX::XMFLOAT3 forward() const	{ DirectX::XMFLOAT3 res; DirectX::XMStoreFloat3(&res, forwardXM()); return res; }
		DirectX::XMFLOAT3 up() const		{ DirectX::XMFLOAT3 res; DirectX::XMStoreFloat3(&res, upXM());		return res; }
		DirectX::XMFLOAT3 right() const		{ DirectX::XMFLOAT3 res; DirectX::XMStoreFloat3(&res, rightXM());	return res; }

		DirectX::XMVECTOR forwardXM() const { return DirectX::XMVector3Normalize(matrix.r[2]); }
		DirectX::XMVECTOR upXM() const		{ return DirectX::XMVector3Normalize(matrix.r[1]); }
		DirectX::XMVECTOR rightXM() const   { return DirectX::XMVector3Normalize(matrix.r[0]); }

		void calculateMatrix()
		{
			matrix =
				DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) *
				DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) *
				DirectX::XMMatrixTranslation(position.x, position.y, position.z);
		}
	};
}