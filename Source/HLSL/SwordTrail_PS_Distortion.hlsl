#include "SwordTrail.hlsli"

Texture2D sceneColor : register(t8);     // シーンの色
Texture2D trailNormalMap : register(t9); // トレイル用法線マップ
Texture2D trailMask : register(t10);     // トレイルのマスク

SamplerState linearSampler : register(s0); // サンプラー


float4 main(TrailVS_OUT pin) : SV_Target
{
    // 厚みを計算する
    float thickness = pin.texcoord.y;
    
    // 中心と端の強さを計算する
    float coreRegion = smoothstep(CORE_REGION_START, CORE_REGION_END, thickness);
    float edgeRegion = 1.0 - abs(thickness - 0.5) * 2.0;
    
    // 内側の歪みを計算する
    float2 innerUV = pin.texcoord * float2(INNER_UV_SCALE_X, INNER_UV_SCALE_Y) + time * INNER_UV_SPEED;
    float2 innerNormal = trailNormalMap.Sample(linearSampler, innerUV).xy;
    innerNormal = innerNormal * NORMAL_SCALE - NORMAL_BIAS;
    
    // 外側の歪みを計算する
    float2 outerUV = pin.texcoord * float2(OUTER_UV_SCALE_X, OUTER_UV_SCALE_Y) + time * OUTER_UV_SPEED;
    float2 outerNormal = trailNormalMap.Sample(linearSampler, outerUV).xy;
    outerNormal = outerNormal * NORMAL_SCALE - NORMAL_BIAS;
    
    // 歪みを合成する
    float2 combinedNormal = lerp(outerNormal, innerNormal, coreRegion);
    combinedNormal = combinedNormal * COMBINED_NORMAL_SCALE - COMBINED_NORMAL_BIAS;
    
    // 画面を歪ませる
    float2 distortedUV = pin.screenUV + combinedNormal * DISTORTION_STRENGTH;
    float3 distortedScene = sceneColor.Sample(linearSampler, distortedUV).rgb;
    
    // 色を設定する
    float3 coreColor, edgeColor, surfaceGlow;

    if (isPlayer > PLAYER_CHECK)
    {
        coreColor = PLAYER_CORE_COLOR;
        edgeColor = PLAYER_EDGE_COLOR;
        surfaceGlow = PLAYER_SURFACE_GLOW;
    }
    else
    {
        coreColor = ENEMY_CORE_COLOR;
        edgeColor = ENEMY_EDGE_COLOR;
        surfaceGlow = ENEMY_SURFACE_GLOW;
    }

    // 色を補間する
    float3 baseColor = lerp(edgeColor, coreColor, coreRegion);
    
    // エッジの発光を追加する
    float edgeIntensity = (1.0 - coreRegion) * edgeRegion;
    baseColor += surfaceGlow * edgeIntensity * EDGE_INTENSITY_FACTOR;
    
    // シーンと合成する
    float depthBlend = lerp(DEPTH_BLEND_EDGE, DEPTH_BLEND_CORE, coreRegion);
    float3 finalColor = lerp(distortedScene, baseColor, depthBlend);
    
    // 発光を追加する
    float glowIntensity = edgeRegion * GLOW_INTENSITY_FACTOR;
    finalColor += baseColor * glowIntensity;
    
    // マスクを取得する
    float3 maskColor = trailMask.Sample(linearSampler, pin.texcoord).rgb;
    float baseAlpha = (maskColor.r + maskColor.g + maskColor.b) / 3.0f;
    
    // 透明度を調整する
    float volumeAlpha = baseAlpha * (BASE_ALPHA_VOLUME + edgeRegion * EDGE_ALPHA_BOOST);
    volumeAlpha *= lerp(CORE_ALPHA_MIN, CORE_ALPHA_MAX, coreRegion);
    
    // 脈動を追加する
    float pulse = sin(time * PULSE_SPEED) * PULSE_STRENGTH + PULSE_BASE;
    volumeAlpha *= pulse;
    
    volumeAlpha = saturate(volumeAlpha);
    
    return float4(finalColor, volumeAlpha);
}