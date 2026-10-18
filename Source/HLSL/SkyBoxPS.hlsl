TextureCube SkyboxTexture : register(t0);
SamplerState Sampler : register(s0);

struct PS_IN
{
    float4 Position : SV_POSITION;
    float3 TexCoord : TEXCOORD;
};

float4 main(PS_IN input) : SV_TARGET
{
    return SkyboxTexture.Sample(Sampler, input.TexCoord);
  
}