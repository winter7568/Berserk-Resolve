#include "Skinning.hlsli"
#include "PBR.hlsli"

PBR_VS_OUT main(
    float4 position : POSITION,
    float4 boneWeights : BONE_WEIGHTS,
    uint4 boneIndices : BONE_INDICES,
    float2 texcoord : TEXCOORD,
    float3 normal : NORMAL,
    float3 tangent : TANGENT)
{
    PBR_VS_OUT vout = (PBR_VS_OUT) 0;

    // スキニング処理を行う
    position = SkinningPosition(position, boneWeights, boneIndices);
    normal = SkinningVector(normal, boneWeights, boneIndices);
    tangent = SkinningVector(tangent, boneWeights, boneIndices);

    // 座標変換を行う
    vout.position = mul(position, viewProjection);
    vout.worldPos = position.xyz;

    // 各値を設定する
    vout.texcoord = texcoord;
    vout.normal = normalize(normal);
    vout.tangent = normalize(tangent);
    vout.bitangent = normalize(cross(normal, tangent));

    // シャドウ用の座標を計算する
    float4 shadow = mul(position, lightViewProjection);
    shadow.xyz /= shadow.w;
    shadow.y = -shadow.y;
    shadow.xy = shadow.xy * 0.5f + 0.5f;
    vout.shadow = shadow.xyz;

    return vout;
}