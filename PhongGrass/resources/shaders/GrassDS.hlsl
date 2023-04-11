#include "ShaderInclude.hlsli"

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor;
	float InsideTessFactor			: SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

[domain("tri")]
TransformedVertex main(HS_CONSTANT_DATA_OUTPUT input, float3 domain : SV_DomainLocation, const OutputPatch<InputVertex, NUM_CONTROL_POINTS> patch)
{
    TransformedVertex output;

    output.worldPos = patch[0].pos * domain.x + patch[1].pos * domain.y + patch[2].pos * domain.z;
    output.svPos = mul(float4(output.worldPos, 1.f), viewProjMatrix);
    output.normal = patch[0].normal * domain.x + patch[1].normal * domain.y + patch[2].normal * domain.z;
    output.uv = patch[0].uv * domain.x + patch[1].uv * domain.y + patch[2].uv * domain.z;
    
	return output;
}
