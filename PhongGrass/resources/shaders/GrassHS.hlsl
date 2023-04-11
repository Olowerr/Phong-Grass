#include "ShaderInclude.hlsli"

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor;
	float InsideTessFactor			: SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<InputVertex, NUM_CONTROL_POINTS> ip,
	uint patchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT output;

    output.EdgeTessFactor[0] = 4.f;
	output.EdgeTessFactor[1] = 4.f;
	output.EdgeTessFactor[2] = 4.f;
	output.InsideTessFactor  = 4.f;

	return output;
}

[domain("tri")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
InputVertex main(InputPatch<InputVertex, NUM_CONTROL_POINTS> ip, uint i : SV_OutputControlPointID)
{
    return ip[i];
}
