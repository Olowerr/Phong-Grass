#include "ShaderInclude.hlsli"

float4 main(TransformedVertex input) : SV_TARGET
{
    return diffuseTexture.Sample(simp, input.uv);
   // return diffuseTexture.Sample(simp, calculateFinalUV(input.uv));
}