#pragma once

#include "Okay/Okay.h"
#include "DirectX/DX11.h"

#include <DirectXMath.h>
#include <string>
#include <vector>

namespace Okay
{
	class Mesh
	{
	public:
		struct MeshInfo
		{
			std::string name;

			std::vector<uint32_t> indices;

			std::vector<DirectX::XMFLOAT3> positions;
			std::vector<DirectX::XMFLOAT2> uvs;
			std::vector<DirectX::XMFLOAT3> normals;
			
			//std::vector<glm::vec3> tangents;
			//std::vector<glm::vec3> biTangents;
		};

		static const uint32_t NumBuffers = 3;
		static const uint32_t Stride[NumBuffers];
		static const uint32_t Offset[NumBuffers];

		Mesh();
		Mesh(const MeshInfo& data);
		Mesh(Mesh&& other) noexcept;
		~Mesh();
		//Mesh& operator=(const Mesh&) = delete;
		//Mesh(const Mesh&) = delete;

		void create(const MeshInfo& data);

		void shutdown();

		inline ID3D11Buffer* const* getBuffers() const;
		inline ID3D11Buffer** getBuffers();
		
		inline ID3D11Buffer* getIndexBuffer() const;
		inline uint32_t getNumIndices() const;
		
		inline const std::string& getName() const;
		inline void setName(std::string_view name);

	private:
		std::string name;

		ID3D11Buffer* vertexBuffers[NumBuffers];
		ID3D11Buffer* indexBuffer;

		uint32_t numIndices;
	};


	ID3D11Buffer* const* Mesh::getBuffers() const { return vertexBuffers; }
	ID3D11Buffer** Mesh::getBuffers() { return vertexBuffers; }
	ID3D11Buffer* Mesh::getIndexBuffer() const { return indexBuffer; }
	uint32_t Mesh::getNumIndices() const { return numIndices; }

	const std::string& Mesh::getName() const { return name; }
	void Mesh::setName(std::string_view name) { this->name = name; }
}