#include "ShaderInclude.hlsli"

TransformedVertex main(InputVertex input, uint instanceID : SV_InstanceID)
{
	TransformedVertex output;

    float4x4 instanceMatrix = instanceTransforms[instanceID];
	
    output.worldPos = mul(float4(input.pos, 1.f), instanceMatrix).xyz;
    output.svPos = mul(float4(output.worldPos, 1.f), viewProjMatrix);

    output.normal = normalize(mul(float4(input.normal, 0.f), instanceMatrix).xyz);
	output.uv = input.uv;

	return output;
}