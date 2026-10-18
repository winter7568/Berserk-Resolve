#include "ParticleSystem.hlsli"

Texture2D colorMap : register(t0);

SamplerState samplerState : register(s0);

float4 main(PS_IN pin) : SV_Target
{
    float4 texColor = colorMap.Sample(samplerState, pin.texcoord);
    return texColor * pin.color;
}