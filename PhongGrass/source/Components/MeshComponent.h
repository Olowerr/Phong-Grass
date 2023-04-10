#pragma once

namespace Okay
{
	struct MeshComponent
	{
		MeshComponent(uint32_t meshIdx = 0u)
			:meshIdx(meshIdx)
		{
		}

		uint32_t meshIdx;
	};
}