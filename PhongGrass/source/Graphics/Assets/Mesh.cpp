#include "Mesh.h"
#include "DirectX/DX11.h"

namespace Okay
{
	const uint32_t Mesh::Stride[] = { sizeof(DirectX::XMFLOAT3), sizeof(DirectX::XMFLOAT2), sizeof(DirectX::XMFLOAT3) };
	const uint32_t Mesh::Offset[] = { 0u, 0u, 0u };

	Mesh::Mesh()
		:name(""), numIndices(0u)
	{
		vertexBuffers[0] = nullptr;
		vertexBuffers[1] = nullptr;
		vertexBuffers[2] = nullptr;
		indexBuffer = nullptr;
	}

	Mesh::Mesh(const MeshInfo& data)
		:vertexBuffers{}, indexBuffer()
	{
		create(data);
	}

	Mesh::Mesh(Mesh&& other) noexcept
		:name(std::move(other.name))
	{
		for (uint32_t i = 0; i < NumBuffers; i++)
		{
			vertexBuffers[i] = other.vertexBuffers[i];
			other.vertexBuffers[i] = nullptr;
		}

		indexBuffer = other.indexBuffer;
		other.indexBuffer = nullptr;

		numIndices = other.numIndices;
		other.numIndices = 0u;
	}

	Mesh::~Mesh()
	{
		shutdown();
	}

	void Mesh::create(const MeshInfo& data)
	{
		shutdown();

		DX11& dx11 = DX11::get();
		dx11.createIndexBuffer(&indexBuffer, data.indices.data(), uint32_t(sizeof(uint32_t) * data.indices.size()));

		dx11.createVertexBuffer(&vertexBuffers[0], data.positions.data(), uint32_t(sizeof(DirectX::XMFLOAT3) * data.positions.size()));
		dx11.createVertexBuffer(&vertexBuffers[1], data.uvs.data(), uint32_t(sizeof(DirectX::XMFLOAT2) * data.uvs.size()));
		dx11.createVertexBuffer(&vertexBuffers[2], data.normals.data(), uint32_t(sizeof(DirectX::XMFLOAT3) * data.normals.size()));

		numIndices = ((uint32_t)data.indices.size());
		name = data.name != "" ? data.name : "New Mesh";
	}

	void Mesh::shutdown()
	{
		DX11_RELEASE(vertexBuffers[0]);
		DX11_RELEASE(vertexBuffers[1]);
		DX11_RELEASE(vertexBuffers[2]);

		DX11_RELEASE(indexBuffer);

		numIndices = 0u;
		name.clear();
	}

	
}
