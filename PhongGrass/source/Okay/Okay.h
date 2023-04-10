#pragma once

#include <cassert>
#include <comdef.h>
#include <string>
#include <fstream>

//#include "glm/glm.hpp"


#ifdef DIST
#define OKAY_ASSERT(condition, devMsg) 
#else
#define OKAY_ASSERT(condition, devMsg)\
if (!(condition))\
{\
	__debugbreak();\
}0

#define PRINT_VEC3_WNAME(vec) printf(#vec " - (%.3f, %.3f, %.3f)\n", vec.x, vec.y, vec.z);
#define PRINT_VEC3(vec)		  printf("(%.3f, %.3f, %.3f)\n", vec.x, vec.y, vec.z);

#endif // DIST


#define DX11_RELEASE(X)		 if (X) { X->Release(); X = nullptr; }
#define OKAY_DELETE(X)		 if (X) { delete X;		X = nullptr; }
#define OKAY_DELETE_ARRAY(X) if (X) { delete[]X;	X = nullptr; }

#define OKAY_VERIFY(X)		if (!X) return false;
#define CHECK_BIT(X, pos)	((X) & 1<<(pos))
#define ARRAY_SIZE(X)		(sizeof(X) / sizeof(X[0]))

#define VEC2_GLM_TO_IMGUI(vec)	ImVec2((float)vec.x, (float)vec.y)
#define UNORM_TO_UCHAR(value)	unsigned char((value) * UCHAR_MAX)
#define UCHAR_TO_UNORM(value)	float((value) / (float)UCHAR_MAX)

#define ENGINE_RESOURCES_PATH "../OkayEngine/engine_resources/"
#define SHADER_PATH ENGINE_RESOURCES_PATH "shaders/"

namespace Okay
{
	constexpr uint32_t INVALID_UINT = ~0u;

	static bool readBinary(std::string_view binPath, std::string& output)
	{
		std::ifstream reader(binPath.data(), std::ios::binary);
		OKAY_VERIFY(reader);

		reader.seekg(0, std::ios::end);
		output.reserve((size_t)reader.tellg());
		reader.seekg(0, std::ios::beg);

		output.assign(std::istreambuf_iterator<char>(reader), std::istreambuf_iterator<char>());

		return true;
	}

	static size_t findLastSlashPos(std::string_view path)
	{
		size_t pos = path.find_last_of('/');
		return pos == std::string_view::npos ? path.find_last_of('\\') : pos;

	}

	static std::string_view getFileEnding(std::string_view path)
	{
		return path.substr(path.find_last_of('.'));
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename... Args>
	static inline Ref<T> createRef(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}