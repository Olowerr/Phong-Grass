#include "ShaderInclude.hlsli"

static float3 SUN_DIR = normalize(float3(1.f, 1.f, 1.f));

static const float MIN = 0.2f;
static const float MAX = 0.8f;

static const float4 COLOURS[5] = 
{
    float4(0.f, 153.f, 0.f, 0.f) / 255.f,
    float4(0.f, 153.f, 51.f, 0.f) / 255.f,
    float4(0.f, 204.f, 0.f, 0.f) / 255.f,
    float4(51.f, 102.f, 0.f, 0.f) / 255.f,
    float4(51.f, 204.f, 51.f, 0.f) / 255.f,
};

float4 main(TransformedGrssVertex input) : SV_TARGET
{
    return COLOURS[(int) input.instanceID % 5];
    
    float z = input.svPos.z / input.svPos.w;
    float4 res = float4(0.f, 1.f, 0.f, 0.f) * z * 5.f;
    
    res *= (MAX - MIN);
    res += float4(MIN, MIN, MIN, MIN);
    
    return res;
    //return float4(0.f, 1.f, 0.f, 0.f) * z * 5.f * max(dot(SUN_DIR, input.normal), 0.4f);
    //return float4(0.f, 1.f, 0.f, 0.f) * max(dot(SUN_DIR, input.normal), 0.f);
}