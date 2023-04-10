#pragma once
#include "Okay/Okay.h"

#include <DirectXMath.h>

namespace Okay
{
	class Material  
	{
	public:

		static const uint32_t BASECOLOUR_INDEX = 0u;
		static const uint32_t SPECULAR_INDEX = 1u;

		struct GPUData
		{
			DirectX::XMFLOAT2 uvTiling = { 1.f, 1.f };
			DirectX::XMFLOAT2 uvOffset = { 0.f, 0.f };
			float shinynessExp = 50.f;
			float padding[3];
		};

		struct Description
		{
			Description() = default;

			std::string_view name;
			uint32_t textureIDs[2]{};
			GPUData gpuData{};
			bool twoSided = false;
		};

		Material();
		Material(const Description& desc);
		Material(Material&& other) noexcept;
		~Material();

		inline const std::string& getName() const;
		inline void setName(std::string_view name);

		inline void setGPUData(const GPUData& data);
		inline GPUData& getGPUData();
		inline const GPUData& getGPUData() const;

		Description getDesc() const;

		inline uint32_t getBaseColour() const;
		inline uint32_t getSpecular() const;

		inline void setBaseColour(uint32_t textureIdx);
		inline void setSpecular(uint32_t textureIdx);

	private:
		std::string name;
		uint32_t textures[2];
		GPUData data;
		bool isTwoSided;
	};


	inline void Material::setName(std::string_view name) { this->name = name; }
	inline const std::string& Material::getName() const  { return name; }

	inline void Material::setGPUData(const Material::GPUData& data) { this->data = data; }
	inline Material::GPUData& Material::getGPUData()				{ return data; }
	inline const Material::GPUData& Material::getGPUData() const	{ return data; }

	inline void Material::setBaseColour(uint32_t textureIdx) { textures[0] = textureIdx; }
	inline void Material::setSpecular(uint32_t textureIdx)	 { textures[1] = textureIdx; }

	inline uint32_t Material::getBaseColour() const { return textures[0]; }
	inline uint32_t Material::getSpecular() const	{ return textures[1]; }
}


/*

	BaseColour
	Specular
	Tiling
	Offset
	Two sided

	Normal
	Emission
	Tessellation
	Opacity
*/