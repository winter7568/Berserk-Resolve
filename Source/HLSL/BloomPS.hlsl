#include "FullScreenQuad.hlsli" 
#include "PostEffect.hlsli" 

Texture2D colorMap : register(t0); // シーンの色バッファ キャラクターと発光を含む
Texture2D luminanceMap : register(t1); // 明るさ用バッファ パーティクル用
SamplerState linearSampler : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    uint width, height;
    luminanceMap.GetDimensions(width, height);
    
    // 2つのバッファから色を取得する
    float4 sceneColor = colorMap.Sample(linearSampler, pin.texcoord);
    float4 luminanceColor = luminanceMap.Sample(linearSampler, pin.texcoord);
    
    // シーンから発光部分を取り出す
    float3 sceneEmissive = 0;
    
    // ピクセルの明るさを計算する 
    float sceneBrightness = dot(sceneColor.rgb, float3(LUMINANCE_R, LUMINANCE_G, LUMINANCE_B));
    
    if (sceneBrightness > EMISSIVE_THRESHOLD)
    {
        // 明るい部分だけを抽出する
        float3 emissiveParts = sceneColor.rgb * (sceneBrightness - EMISSIVE_THRESHOLD);
        
        // ガウシアンブラーでぼかす
        float3 emissiveBlur = 0;
        float gaussianKernelTotal = 0;
        
        [unroll] 
        for (int x = -HALF_KERNEL_SIZE; x <= HALF_KERNEL_SIZE; x += 1)
        {
            [unroll] 
            for (int y = -HALF_KERNEL_SIZE; y <= HALF_KERNEL_SIZE; y += 1)
            {
                float gaussianKernel = exp(-(x * x + y * y) / (2.0 * gaussianSigma * gaussianSigma)) /
                                       (TAU * gaussianSigma * gaussianSigma);
                emissiveBlur += emissiveParts * gaussianKernel;
                gaussianKernelTotal += gaussianKernel;
            }
        }
        emissiveBlur /= gaussianKernelTotal;
        
        sceneEmissive = emissiveBlur;
    }
    
    // パーティクルのブラー処理を行う
    float3 particleBlur = 0;
    float particleKernelTotal = 0;
    
    [unroll] 
    for (int px = -HALF_KERNEL_SIZE; px <= HALF_KERNEL_SIZE; px += 1)
    {
        [unroll] 
        for (int py = -HALF_KERNEL_SIZE; py <= HALF_KERNEL_SIZE; py += 1)
        {
            float gaussianKernel = exp(-(px * px + py * py) / (2.0 * gaussianSigma * gaussianSigma)) /
                                   (TAU * gaussianSigma * gaussianSigma);
            particleBlur += luminanceMap.Sample(linearSampler,
                pin.texcoord + float2(px * 1.0 / width, py * 1.0 / height)).rgb * gaussianKernel;
            particleKernelTotal += gaussianKernel;
        }
    }
    particleBlur /= particleKernelTotal;
    
    // 発光を合成する
    float3 totalBloom = sceneEmissive + particleBlur;
    
    // 最終的な色を計算する
    // シーンにトーンマッピングを適用する (Reinhard-style tone mapping)
    float3 tonemapped = 1.0f - exp(-sceneColor.rgb * EXPOSURE);
    
    // ブルームを加える
    float3 bloom = totalBloom * bloomIntensity;
    
    // パーティクルが見えるように合成する
    float3 finalColor = max(tonemapped + bloom, luminanceColor.rgb);
    
    // 画面フェードを適用する
    finalColor = finalColor * (1.0f - screenFadeAlpha);
    
    return float4(finalColor, sceneColor.a);
}