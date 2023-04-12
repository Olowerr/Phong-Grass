#include "ShaderInclude.hlsli"

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor;
	float InsideTessFactor			: SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<GrassVertex, NUM_CONTROL_POINTS> ip,
	uint patchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT output;
    
    const float distance = length(instanceTransforms[ip[0].instanceID][3].xyz - camPos);
    const float tessellationFactor = pow(1.f - (distance / maxGrassAppliedDistance), tessGrassFactorExponent);
    
    output.EdgeTessFactor[0] = max(tessellationFactor * maxGrassTessFactor, 1.f);
	output.EdgeTessFactor[1] = max(tessellationFactor * maxGrassTessFactor, 1.f);
	output.EdgeTessFactor[2] = max(tessellationFactor * maxGrassTessFactor, 1.f);
    output.InsideTessFactor = max(tessellationFactor * maxGrassTessFactor, 1.f);

	return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
InputVertex main(InputPatch<GrassVertex, NUM_CONTROL_POINTS> ip, uint i : SV_OutputControlPointID)
{
    InputVertex vertex;
    vertex.pos = ip[i].pos;
    vertex.uv = ip[i].uv;
    vertex.normal = ip[i].normal;

    return vertex;
}
