cbuffer MatrixBuffer : register(b0)
{
    matrix ViewProj;
};

struct VS_IN
{
    float3 Position : POSITION;
};

struct VS_OUT
{
    float4 Position : SV_POSITION;
    float3 TexCoord : TEXCOORD;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    output.Position = mul(float4(input.Position, 1.0), ViewProj).xyww;
    output.TexCoord = input.Position;
    return output;
}