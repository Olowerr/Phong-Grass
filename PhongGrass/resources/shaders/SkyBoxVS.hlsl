#include "ShaderInclude.hlsli"

struct Output 
{
	float4 pos : SV_POSITION;
	float3 localPos : LOCAL_POS;
};

Output main(float3 pos : POSITION)
{
	Output output;
	output.localPos = pos;
	output.pos = mul(float4(camPos + pos, 1.f), viewProjMatrix).xyww;

	return output;
}