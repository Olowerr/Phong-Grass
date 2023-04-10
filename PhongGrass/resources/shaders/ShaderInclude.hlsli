
struct InputVertex
{
    float3 pos : POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
};

struct TransformedVertex
{
    float4 svPos : SV_POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;

    float3 worldPos : W_POS;
};

cbuffer renderData : register(b0)
{
    float4x4 viewProjMatrix;
    
    float3 camPos;
    float pad0;
    
    float3 camDir;
    float pad1;
}

cbuffer objectData : register(b1)
{
    float4x4 worldMatrix;
    float2 uvOffset;
    float2 uvTiling;
};

float2 calculateFinalUV(float2 inputUV)
{
    return (inputUV + uvOffset) * uvTiling;
}

// Resources
Texture2D diffuseTexture : register(t0);
StructuredBuffer<float4x4> instanceTransforms : register(t1);

// Samplers
SamplerState simp : register(s0);