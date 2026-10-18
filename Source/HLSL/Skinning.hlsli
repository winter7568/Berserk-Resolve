// ボーンの変換行列をまとめた定数バッファ
cbuffer CbSkeleton : register(b2)
{
    row_major float4x4 boneTransforms[256]; // 最大256個のボーン
}

// 頂点位置にスキニングを適用する
float4 SkinningPosition(float4 position, float4 boneWeights, uint4 boneIndices)
{
    float4 p = float4(0, 0, 0, 0);

    // 影響する4つのボーンを処理する
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        // ボーン行列で変換してウェイトで合成する
        p += (boneWeights[i] * mul(position, boneTransforms[boneIndices[i]]));
    }

    // 最終的な頂点位置を返す
    return p;
}

// ベクトルにスキニングを適用する
float3 SkinningVector(float3 vec, float4 boneWeights, uint4 boneIndices)
{
    float3 v = float3(0, 0, 0);
 
    // 影響する4つのボーンを処理する
    [unroll] 
    for (int i = 0; i < 4; i++)
    {
        // 各ボーンで変換してウェイトで合成する
        v += boneWeights[i] * mul(float4(vec, 0), boneTransforms[boneIndices[i]]).xyz;
    }

    // 最終的なベクトルを返す
    return v;
}