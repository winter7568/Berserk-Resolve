#include "Skinning.hlsli" 
 
cbuffer CbScene : register(b0)
{
    row_major float4x4 lightViewProjection;
}; 
 
float4 main(
 float4 position : POSITION,
 float4 boneWeights : BONE_WEIGHTS,
 uint4 boneIndices : BONE_INDICES
 ) : SV_POSITION
{
 // ボーンの情報を使って頂点の位置を動かす
    position = SkinningPosition(position, boneWeights, boneIndices);

    // ライトの空間に変換する（シャドウ用）
    return mul(position, lightViewProjection);
}