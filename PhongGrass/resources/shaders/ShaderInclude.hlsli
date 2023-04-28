
struct InputVertex
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct TransformedVertex
{
    float4 svPos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;

    float3 worldPos : W_POS;
};

struct GrassVertex
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    uint instanceID : INSTANCE_ID;
};

struct TransformedGrssVertex
{
    float4 svPos : SV_POSITION;
    float3 pos : POSITION;
    float3 normal : NORMAL;
    
    uint instanceID : INSTANCE_ID;
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
    float shinyness;
    float3 pad3;
};

cbuffer grassData : register(b2)
{
    float maxGrassTessFactor;
    float maxGrassAppliedDistance;
    float tessGrassFactorExponent;
    float pad2;
}

float2 calculateFinalUV(float2 inputUV)
{
    return (inputUV + uvOffset) * uvTiling;
}

// Resources
Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
StructuredBuffer<float4x4> instanceTransforms : register(t2);
TextureCube skyBoxTexture : register(t3);

// Samplers
SamplerState simp : register(s0);