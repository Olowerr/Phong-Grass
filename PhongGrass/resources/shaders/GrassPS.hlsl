#include "ShaderInclude.hlsli"

static float3 SUN_DIR = normalize(float3(1.f, 1.f, 1.f));

float4 main(TransformedVertex input) : SV_TARGET
{
    return float4(0.f, 1.f, 0.f, 1.f);
    return float4(input.normal, 1.f);
    return float4(0.f, 1.f, 0.f, 0.f) * max(dot(SUN_DIR, input.normal), 0.f);
}