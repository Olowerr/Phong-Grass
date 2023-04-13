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
    
    const float distance = length(instanceTransforms[ip[0].instanceID][3].xyz - float3(camPos.x, 0.f, camPos.z));
    const float tessellationFactor = pow(1.f - (distance / maxGrassAppliedDistance), tessGrassFactorExponent);
    const float clampedMaxFactor = max(tessellationFactor * maxGrassTessFactor, 1.f);
    //const float clampedMaxFactor = tessellationFactor * maxGrassTessFactor;
    //const float clampedMaxFactor = 0.1f;
    
    output.EdgeTessFactor[0] = output.EdgeTessFactor[1] = output.EdgeTessFactor[2] = clampedMaxFactor * 2.5f;
    output.InsideTessFactor = clampedMaxFactor;

        //output.InsideTessFactor = tessellationFactor * maxGrassTessFactor;
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
