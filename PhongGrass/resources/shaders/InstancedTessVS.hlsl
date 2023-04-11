#include "ShaderInclude.hlsli"

InputVertex main(InputVertex vertexData, uint instanceID : SV_InstanceID)
{
    float4x4 instanceWorldMatrix = instanceTransforms[instanceID];
	
    vertexData.pos = mul(float4(vertexData.pos, 1.f), instanceWorldMatrix).xyz;
    vertexData.normal = normalize(mul(float4(vertexData.normal, 0.f), instanceWorldMatrix).xyz);
    vertexData.uv = vertexData.uv;

    return vertexData;
}