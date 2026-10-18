#include "FullScreenQuad.hlsli" 
#include "PostEffect.hlsli" 

Texture2D colorMap : register(t0);
SamplerState linearSampler : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    // 描画されたシーンの色を取得する
    float4 color = colorMap.Sample(linearSampler, pin.texcoord);
 
    // 明るい部分だけを取り出す
    // ブルーム用の処理で明るいピクセルだけ残し暗い部分は弱くする
    float luminance = dot(color.rgb, float3(LUMINANCE_R, LUMINANCE_G, LUMINANCE_B));
    color.rgb *= smoothstep(luminanceExtractionLowerEdge, luminanceExtractionHigherEdge, luminance);
    return color;
}