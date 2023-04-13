#pragma once

namespace Okay
{
	struct MeshComponent
	{
		MeshComponent() = default;
		MeshComponent(uint32_t meshIdx, uint32_t materialIdx)
			:meshIdx(meshIdx), materialIdx(materialIdx)
		{
		}

		uint32_t meshIdx = 0u;
		uint32_t materialIdx = 0u;
	};
}