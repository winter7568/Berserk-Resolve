#include "FullScreenQuad.hlsli" 
#include "PostEffect.hlsli" 

Texture2D colorMap : register(t0); // シーンの色
Texture2D vignetteTexture : register(t1); // ビネット用テクスチャ

SamplerState linearSampler : register(s0); // サンプラー

float4 main(VS_OUT pin) : SV_TARGET
{
    // シーンの色を取得する
    float4 color = colorMap.Sample(linearSampler, pin.texcoord);
    
    // ビネットが有効な場合のみ適用する
    if (vignetteEnabled > VIGNETTE_ENABLE_CHECK)
    {
        // ビネット用のUVを計算する
        float2 vignetteUV = (pin.texcoord - VIGNETTE_UV_CENTER) * vignetteScale + VIGNETTE_UV_CENTER;
        
        // ビネットマスクを取得する
        float vignetteMask = vignetteTexture.Sample(linearSampler, vignetteUV).r;
        
        // 強さを調整する
        vignetteMask = lerp(DEFAULT_MASK, vignetteMask, vignetteIntensity);
        
        // 色を少し加える
        color.rgb = lerp(color.rgb, VIGNETTE_COLOR, vignetteMask * VIGNETTE_COLOR_STRENGTH);
        
        // ビネットを適用する
        color.rgb *= vignetteMask;
    }
    
    return color;
}