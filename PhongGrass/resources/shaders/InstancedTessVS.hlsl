#include "ShaderInclude.hlsli"

GrassVertex main(InputVertex vertexData, uint instanceID : SV_InstanceID)
{
    GrassVertex output;
    output.instanceID = instanceID;
    
    float4x4 instanceWorldMatrix = instanceTransforms[instanceID];
	
    output.pos = mul(float4(vertexData.pos, 1.f), instanceWorldMatrix).xyz;
    output.normal = normalize(mul(float4(vertexData.normal, 0.f), instanceWorldMatrix).xyz);
    output.uv = vertexData.uv;

    return output;
}