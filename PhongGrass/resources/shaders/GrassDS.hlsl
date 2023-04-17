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

// 1/2 for phongGrass | 1 for phongGrass1
static const float shapeFactor = 0.5f;

[domain("tri")]
TransformedGrssVertex main(HS_CONSTANT_DATA_OUTPUT input, float3 domain : SV_DomainLocation, const OutputPatch<GrassVertex, NUM_CONTROL_POINTS> patch)
{
    TransformedGrssVertex output;

    output.instanceID = patch[0].instanceID;
    output.pos = patch[0].pos * domain.x + patch[1].pos * domain.y + patch[2].pos * domain.z;

    
#if 0
    float3 res = 
    { 
        domain.x * orthogonalProjection(output.pos, patch[0].pos, patch[0].normal) +
        domain.y * orthogonalProjection(output.pos, patch[1].pos, patch[1].normal) +
        domain.z * orthogonalProjection(output.pos, patch[2].pos, patch[2].normal)
    };
    output.worldPos = (1.f - shapeFactor) * output.pos + shapeFactor * res;
#else
    float3x3 res =
    {
        orthogonalProjection(output.pos, patch[0].pos, patch[0].normal),
        orthogonalProjection(output.pos, patch[1].pos, patch[1].normal),
        orthogonalProjection(output.pos, patch[2].pos, patch[2].normal)
    };
    output.pos = (1.f - shapeFactor) * output.pos + mul(domain * shapeFactor, res);
#endif
    
    output.svPos = mul(float4(output.pos, 1.f), viewProjMatrix);
    
	return output;
}
