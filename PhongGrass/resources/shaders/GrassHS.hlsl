#include "ShaderInclude.hlsli"

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor;
	float InsideTessFactor			: SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(InputPatch<GrassVertex, NUM_CONTROL_POINTS> ip, uint patchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT output;
    
    const float3 camToGrass = instanceTransforms[ip[0].instanceID][3].xyz - float3(camPos.x, 0.f, camPos.z);
    const float distance = length(camToGrass);

#if MODE == 0
    const float tessellationFactor = (1.f - distance / maxGrassAppliedDistance) * maxGrassTessFactor + 1.f;
    const float clampedMaxFactor = max(tessellationFactor, 1.f);
#elif MODE == 1
    const float tessellationFactor = maxGrassTessFactor + pow(distance / maxGrassAppliedDistance, tessGrassFactorExponent) * -maxGrassTessFactor + 1.f;
    const float clampedMaxFactor = max(tessellationFactor, 1.f);
#elif MODE == 2
    const float tessellationFactor = pow(1.f - distance / maxGrassAppliedDistance, tessGrassFactorExponent) * maxGrassTessFactor * (distance < maxGrassAppliedDistance) + 1.f;
    const float clampedMaxFactor = max(tessellationFactor, 1.f);
#else
    const float clampedMaxFactor = 0.f;
#endif
    
    
    output.EdgeTessFactor[0] = output.EdgeTessFactor[1] = output.EdgeTessFactor[2] =
        output.InsideTessFactor = clampedMaxFactor;
    
	return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
GrassVertex main(InputPatch<GrassVertex, NUM_CONTROL_POINTS> ip, uint i : SV_OutputControlPointID)
{
    GrassVertex vertex;
    vertex.pos = ip[i].pos;
    vertex.normal = ip[i].normal;
    vertex.instanceID = ip[i].instanceID;

    return vertex;
}
