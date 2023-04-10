#include "ContentBrowser.h"
#include "Assets/ImpExp/stb_image.h"

namespace Okay
{
	ContentBrowser::ContentBrowser()
	{
	}

	ContentBrowser::~ContentBrowser()
	{
	}

	bool ContentBrowser::importFile(std::string_view path)
	{
		const std::string_view fileEnding = getFileEnding(path);

		if (ContentBrowser::canLoadTexture(path.data()))
			return loadTexture(path.data());

		// TODO: "Ask" assimp if it can load the file instead
		if (fileEnding == ".fbx" || fileEnding == ".FBX" || fileEnding == ".obj" || fileEnding == ".OBJ")
			return loadMesh(path);

		return false;
	}

	bool ContentBrowser::loadMesh(std::string_view path)
	{
		std::string materialName;
		std::string texturePaths[2];

		Okay::Mesh::MeshInfo meshInfo;
		OKAY_VERIFY(Importer::Load(path, meshInfo, texturePaths, materialName));
		
		const size_t pos = findLastSlashPos(path);
		const std::string location(pos == std::string_view::npos ? "" : path.substr(0ull, pos + 1ull));

		// TODO: Add option to skip importing the material
		Material::Description matDesc;
		matDesc.name = materialName;

		std::string fileName;
		fileName.reserve(64ull);
		for (uint32_t i = 0; i < 2u; i++)
		{
			bool found = false;
			
			const size_t dotPos = texturePaths[i].find_last_of('.');
			fileName.assign(texturePaths[i].c_str(), dotPos == std::string_view::npos ? texturePaths[i].size() : dotPos);

			for (size_t j = 0; j < textures.size() && !found; j++)
			{
				if (textures[j].getName() == fileName)
				{
					matDesc.textureIDs[i] = (uint32_t)j;
					found = true;
				}
			}

			if (!found)
				matDesc.textureIDs[i] = loadTexture(location + texturePaths[i]) ? (uint32_t)textures.size() - 1u : 0u;
		}
		
		materials.emplace_back(matDesc);

		meshInfo.materialId = (uint32_t)materials.size() - 1u;
		meshes.emplace_back(meshInfo);
		
		return true;

	}

	bool ContentBrowser::loadTexture(std::string_view path)
	{
		int x, y, c;
		unsigned char* pData = stbi_load(path.data(), &x, &y, &c, 4);
		OKAY_VERIFY(pData);

		size_t pos = findLastSlashPos(path);
		pos = pos == std::string_view::npos ? 1ull : ++pos;

		// texName contains ex: .png, it is removed inside the Texture constructor
		std::string_view texName = path.substr(pos);
		textures.emplace_back(pData, (uint32_t)x, (uint32_t)y, texName);

		stbi_image_free(pData);

		return true;
	}
	
	bool ContentBrowser::canLoadTexture(const char* path)
	{
		int x, y, c;
		return stbi_info(path, &x, &y, &c);
	}
}