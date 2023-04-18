#pragma once
#include "Okay/Okay.h"

#include <DirectXMath.h>
#include <d3d11.h>
#include <functional>

namespace Okay
{
	class RenderTexture
	{
	public:
		enum BitPos : uint32_t
		{
			B_RENDER		 = 0,
			B_SHADER_READ	 = 1,
			B_SHADER_WRITE	 = 2,
			B_DEPTH			 = 3,
		};

		enum Format : uint32_t
		{
			INVALID,
			F_8X1,
			F_8X4,
			F_32X4,
		};

		enum Flags : uint32_t
		{
			RENDER			= 1 << BitPos::B_RENDER,
			SHADER_READ		= 1 << BitPos::B_SHADER_READ,
			SHADER_WRITE	= 1 << BitPos::B_SHADER_WRITE,
			DEPTH			= 1 << BitPos::B_DEPTH,
		};

		RenderTexture();
		RenderTexture(ID3D11Texture2D* texture, uint32_t flags);
		RenderTexture(uint32_t width, uint32_t height, uint32_t flags, Format format = Format::F_8X4);
		~RenderTexture();
		void shutdown();

		void create(ID3D11Texture2D* texture, uint32_t flags);
		void create(uint32_t width, uint32_t height, uint32_t flags, Format format = Format::F_8X4);

		void clear();
		
		template<typename Func, typename T>
		inline void addOnResizeCallback(Func&& func, T* pInstance);

		template<typename Func>
		inline void removeOnResizeCallback(Func&& func, void* pOwner);

		void resize(uint32_t width, uint32_t height);
		inline DirectX::XMUINT2 getDimensions() const;

		inline uint32_t getFlags() const;
		inline bool valid() const;

		inline ID3D11Texture2D* getBuffer() const;
		inline ID3D11RenderTargetView* const* getRTV() const;
		inline ID3D11ShaderResourceView* const* getSRV() const;
		inline ID3D11UnorderedAccessView* const* getUAV() const;

		inline ID3D11Texture2D* getDepthBuffer();
		inline ID3D11DepthStencilView* const* getDSV() const;

	private:
		std::vector<std::function<void(uint32_t, uint32_t)>> callbacks;
		
		DirectX::XMUINT2 dims;
		uint32_t flags;
		Format format;

		ID3D11Texture2D* buffer;
		ID3D11RenderTargetView* rtv;
		ID3D11ShaderResourceView* srv;
		ID3D11UnorderedAccessView* uav;

		ID3D11Texture2D* depthBuffer;
		ID3D11DepthStencilView* dsv;

		void readFlgs(uint32_t flags);
	};

	template<typename Func, typename T>
	inline void RenderTexture::addOnResizeCallback(Func&& func, T* pInstance)
	{
		callbacks.emplace_back(std::bind(func, pInstance, std::placeholders::_1, std::placeholders::_2));
	}

	template<typename Func>
	inline void RenderTexture::removeOnResizeCallback(Func&& func, void* pOwner)
	{
		for (auto it = callbacks.begin(); it != callbacks.end(); ++it)
		{
			if ((void*)it->target<void*>() == pOwner)
			{
				callbacks.erase(it);
				return;
			}
		}
	}

	inline uint32_t RenderTexture::getFlags() const { return flags; }
	inline bool RenderTexture::valid() const		{ return buffer; }

	inline ID3D11Texture2D* RenderTexture::getBuffer() const				{ return buffer; }
	inline ID3D11RenderTargetView* const* RenderTexture::getRTV() const		{ return &rtv; }
	inline ID3D11ShaderResourceView* const* RenderTexture::getSRV() const	{ return &srv; }
	inline ID3D11UnorderedAccessView* const* RenderTexture::getUAV() const	{ return &uav; }

	inline ID3D11Texture2D* RenderTexture::getDepthBuffer()				{ return depthBuffer; }
	inline ID3D11DepthStencilView* const* RenderTexture::getDSV() const { return &dsv; }

	inline DirectX::XMUINT2 RenderTexture::getDimensions() const		{ return dims; }
} // Okay