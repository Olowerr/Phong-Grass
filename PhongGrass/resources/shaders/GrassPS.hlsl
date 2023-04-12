#include "ShaderInclude.hlsli"

static float3 SUN_DIR = normalize(float3(1.f, 1.f, 1.f));

float4 main(TransformedVertex input) : SV_TARGET
{
    float z = input.svPos.z / input.svPos.w;
    return float4(0.f, 1.f, 0.f, 0.f) * z * 5.f;
    //return float4(0.f, 1.f, 0.f, 0.f) * z * 5.f * max(dot(SUN_DIR, input.normal), 0.4f);
    //return float4(0.f, 1.f, 0.f, 0.f) * max(dot(SUN_DIR, input.normal), 0.f);
}