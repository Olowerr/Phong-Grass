#include "ShaderInclude.hlsli"

struct Input
{
	float4 pos : SV_POSITION;
	float3 localPos : LOCAL_POS;
};

float4 main(Input input) : SV_TARGET
{
    return skyBoxTexture.Sample(simp, input.localPos);
}