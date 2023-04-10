#pragma once
#include <string>
#include <unordered_map>
#include <memory>

#include "Okay/Okay.h"
#include "Assets/ImpExp/OkayImporter.h"
#include "Assets/Texture.h"
#include "Assets/Material.h"

// TODO: Move STBI to deps

namespace Okay
{
	class ContentBrowser // TODO: Clean up 
	{
	private:
		ContentBrowser();
	public:
		ContentBrowser(const ContentBrowser&) = delete;
		ContentBrowser(ContentBrowser&&) = delete;
		ContentBrowser& operator=(const ContentBrowser&) = delete;
		~ContentBrowser();

		static inline ContentBrowser& get()
		{
			static ContentBrowser cb;
			return cb;
		}

		// Const functions are kinda weird
		// cuz why would you ever have a const-version of this? It's a singleton..
		// Rmv?


		static bool canLoadTexture(const char* path);
		bool importFile(std::string_view path);

		template<typename Asset, typename... Args>
		Asset& addAsset(Args&&... args);

		template<typename Asset>
		Asset& getAsset(uint32_t index);

		template<typename Asset>
		const Asset& getAsset(uint32_t index) const;

		template<typename Asset>
		uint32_t getAmount() const;

		template<typename Asset>
		const std::vector<Asset>& getAll() const;

		template<typename Asset, typename Function, typename... Args>
		void forEachAsset(Function function, Args&&... args);

		template<typename Asset>
		uint32_t getAssetID(std::string_view assetName) const;

	private:
		std::vector<Mesh> meshes;
		std::vector<Texture> textures;
		std::vector<Material> materials;

		bool loadMesh(std::string_view path);
		bool loadTexture(std::string_view path);

		template<typename Asset>
		std::vector<Asset>& getAssets();

		template<typename Asset>
		const std::vector<Asset>& getAssetsConst() const;
	};

#define STATIC_ASSERT_ASSET_TYPE()\
static_assert(std::is_same<Asset, Mesh>() ||\
			  std::is_same<Asset, Texture>() ||\
			  std::is_same<Asset, Material>(), \
			  "Invalid Asset type")

	template<typename Asset>
	std::vector<Asset>& ContentBrowser::getAssets()
	{
		STATIC_ASSERT_ASSET_TYPE();

		if		constexpr (std::is_same<Asset, Mesh>())		return meshes;
		else if constexpr (std::is_same<Asset, Texture>())	return textures;
		else if constexpr (std::is_same<Asset, Material>())	return materials;
	}

	template<typename Asset>
	const std::vector<Asset>& ContentBrowser::getAssetsConst() const
	{
		STATIC_ASSERT_ASSET_TYPE();
		return const_cast<ContentBrowser*>(this)->getAssets<Asset>();
	}

	template<typename Asset, typename ...Args>
	inline Asset& ContentBrowser::addAsset(Args && ...args)
	{
		STATIC_ASSERT_ASSET_TYPE();
		return getAssets<Asset>().emplace_back(args...);
	}

	template<typename Asset>
	inline Asset& ContentBrowser::getAsset(uint32_t index)
	{
		STATIC_ASSERT_ASSET_TYPE();
		std::vector<Asset>& assets = getAssets<Asset>();

		OKAY_ASSERT(index < (uint32_t)assets.size(), "Invalid index");
		return assets[index];
	}

	template<typename Asset>
	inline const Asset& ContentBrowser::getAsset(uint32_t index) const
	{
		// I'd wanna just return the non-const version but I'm it's spooky since it looks recursive

		STATIC_ASSERT_ASSET_TYPE();
		const std::vector<Asset>& assets = getAssets<Asset>();

		OKAY_ASSERT(index < (uint32_t)assets.size(), "Invalid index");
		return assets[index];
	}

	template<typename Asset>
	inline uint32_t ContentBrowser::getAmount() const
	{
		STATIC_ASSERT_ASSET_TYPE();
		return (uint32_t)getAssetsConst<Asset>().size();
	}

	template<typename Asset>
	inline const std::vector<Asset>& ContentBrowser::getAll() const
	{
		STATIC_ASSERT_ASSET_TYPE();
		return getAssetsConst<Asset>();
	}

	template<typename Asset, typename Function, typename ...Args>
	inline void ContentBrowser::forEachAsset(Function function, Args && ...args)
	{
		STATIC_ASSERT_ASSET_TYPE();
		std::vector<Asset>& assets = getAssets<Asset>();
		for (size_t i = 0; i < assets.size(); i++)
			function(assets[i], args...);
	}

	template<typename Asset>
	inline uint32_t ContentBrowser::getAssetID(std::string_view assetName) const
	{
		STATIC_ASSERT_ASSET_TYPE();
		const std::vector<Asset>& assets = getAssetsConst<Asset>();
		for (size_t i = 0; i < assets.size(); i++)
		{
			if (assets[i].getName() == assetName)
				return (uint32_t)i;
		}
		
		return INVALID_UINT;
	}
}