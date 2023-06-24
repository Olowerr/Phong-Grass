#include "ShaderInclude.hlsli"

static const float3 SUN_DIR = normalize(float3(1.f, 1.f, -1.f));
static const float SUN_INTENSITY = 1.f;

static const float gamma = 1.f / 2.2f;
static const float4 gamma4 = float4(gamma, gamma, gamma, gamma);

float4 main(TransformedVertex input) : SV_TARGET
{
    const float4 baseColour = diffuseTexture.Sample(simp, input.uv);
    return baseColour;
    const float4 specularColour = specularTexture.Sample(simp, input.uv);
    const float3 posToCam = normalize(camPos - input.worldPos);
    
    const float4 ambientLight = baseColour * 0.3f;
    const float4 diffuseLight = baseColour * max(dot(input.normal, SUN_DIR), 0.f) * SUN_INTENSITY;
    const float4 specularLight = specularColour * pow(max(dot(reflect(-SUN_DIR, input.normal), posToCam), 0.f), shinyness) * SUN_INTENSITY;
    
    //return saturate(baseColour * (ambientLight + diffuseLight + specularLight));
    return saturate(pow(baseColour * (ambientLight + diffuseLight + specularLight), gamma4));
}