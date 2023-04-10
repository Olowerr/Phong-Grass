#include "ShaderInclude.hlsli"

TransformedVertex main(InputVertex input)
{
	TransformedVertex output;

	output.worldPos = mul(float4(input.pos, 1.f), worldMatrix).xyz;
    output.svPos = mul(float4(output.worldPos, 1.f), viewProjMatrix);

	output.normal = normalize(mul(float4(input.normal, 0.f), worldMatrix).xyz);
	output.uv = input.uv;

	return output;
}