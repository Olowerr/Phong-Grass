#include "ShaderInclude.hlsli"

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor;
	float InsideTessFactor			: SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

float3 orthogonalProjection(float3 position, float3 planePos, float3 planeNormal)
{
    return position - dot(position - planePos, planeNormal) * planeNormal;
}

[domain("tri")]
TransformedVertex main(HS_CONSTANT_DATA_OUTPUT input, float3 domain : SV_DomainLocation, const OutputPatch<InputVertex, NUM_CONTROL_POINTS> patch)
{
    TransformedVertex output;

    output.worldPos = patch[0].pos * domain.x + patch[1].pos * domain.y + patch[2].pos * domain.z;
    output.normal = normalize(patch[0].normal * domain.x + patch[1].normal * domain.y + patch[2].normal * domain.z);
    output.uv = patch[0].uv * domain.x + patch[1].uv * domain.y + patch[2].uv * domain.z;
    
    float shapeFactor = 3.f / 4.f;
    
#if 0
    float3 res = 
    { 
        domain.x * orthogonalProjection(output.worldPos, patch[0].pos, patch[0].normal) +
        domain.y * orthogonalProjection(output.worldPos, patch[1].pos, patch[1].normal) +
        domain.z * orthogonalProjection(output.worldPos, patch[2].pos, patch[2].normal)
    };
    output.worldPos = (1.f - shapeFactor) * output.worldPos + shapeFactor * res;
#else
    float3x3 res =
    {
        orthogonalProjection(output.worldPos, patch[0].pos, patch[0].normal),
        orthogonalProjection(output.worldPos, patch[1].pos, patch[1].normal),
        orthogonalProjection(output.worldPos, patch[2].pos, patch[2].normal)
    };
    output.worldPos = (1.f - shapeFactor) * output.worldPos + mul(domain * shapeFactor, res);
#endif
    
    output.svPos = mul(float4(output.worldPos, 1.f), viewProjMatrix);
    
	return output;
}
