#include "ShaderInclude.hlsli"

static const float4 COLOURS[3] = 
{
    float4(115.f, 214.f, 68.f, 0.f) / 255.f,
    float4(26.f, 78.f, 25.f, 0.f) / 255.f,
    float4(143.f, 255.f, 0.f, 0.f) / 255.f
};

static const float GRASS_HEIGHT = 0.736f;
static const float MODIFIER = 1.25f;

float4 main(TransformedGrssVertex input) : SV_TARGET
{
    return COLOURS[0] * lerp(COLOURS[1], COLOURS[2], (input.pos.y / GRASS_HEIGHT) * MODIFIER);
}