#include "ShaderInclude.hlsli"

GrassVertex main(float3 position : POSITION, float3 normal : NORMAL, uint instanceID : SV_InstanceID)
{
    GrassVertex output;
    
    const float4x4 worldMatrix = instanceTransforms[instanceID];
    
    output.pos = mul(float4(position, 1.f), worldMatrix).xyz;
    output.normal = mul(float4(normal, 0.f), worldMatrix).xyz;
    output.instanceID = instanceID;

    return output;
}