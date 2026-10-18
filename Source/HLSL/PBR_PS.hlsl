#include "PBR.hlsli"
#include "PBRFunctions.hlsli"

// テクスチャ
Texture2D diffuseMap : register(t0);     // ベースカラー
Texture2D normalMap : register(t1);      // 法線マップ
Texture2D emissiveMap : register(t2);    // 発光マップ
Texture2D occlusionMap : register(t3);   // AOマップ
Texture2D metalnessMap : register(t4);   // メタリックマップ
Texture2D RoughnessMap : register(t5);   // ラフネスマップ
Texture2D shadowMap : register(t6);      // シャドウマップ
Texture2D environmentMap : register(t7); // 環境マップ
Texture2D dissolveMask : register(t8);   // ディゾルブ用マスク

// サンプラー
SamplerState linearSampler : register(s0);           // 通常サンプラー
SamplerComparisonState shadowSampler : register(s1); // シャドウ用サンプラー
SamplerState dissolveSampler : register(s2);         // ディゾルブ用サンプラー


float3 IBL(float3 worldNormal, float3 viewDir, float roughness, float3 F0)
{
    // 反射方向を計算する
    float3 R = reflect(-viewDir, worldNormal);

    // 環境マップ用のUVを計算する
    float2 uv;
    uv.x = (R.x * HALF_SIZE + HALF_SIZE) * HALF_SIZE;
    if (R.z < 0)
    {
        uv.x = 1.0 - uv.x;
    }
    uv.y = -R.y * HALF_SIZE + HALF_SIZE;

    // 環境マップをサンプリングする
    float3 envColor = environmentMap.Sample(linearSampler, uv).rgb;

    // 明るさを調整する
    envColor *= materialEnvironmentStrength;
    envColor = min(envColor, MAX_ENV_COLOR);

    // スペキュラ反射を計算する
    float specularIBL = saturate(1.0 - roughness);
    float3 specular = envColor * lerp(F0, float3(1.0, 1.0, 1.0), specularIBL);

    return specular;
}

float4 main(PBR_VS_OUT pin) : SV_Target
{
    // 各テクスチャから値を取得する
    float4 albedoSample = diffuseMap.Sample(linearSampler, pin.texcoord);
    float3 normalSample = normalMap.Sample(linearSampler, pin.texcoord).rgb;
    float4 metalRoughSample = metalnessMap.Sample(linearSampler, pin.texcoord);
    float ao = occlusionMap.Sample(linearSampler, pin.texcoord).r;
    float3 emissiveSample = emissiveMap.Sample(linearSampler, pin.texcoord).rgb;
    
    // マテリアルの値を設定する
    float3 baseColor = pow(saturate(albedoSample.rgb), GAMMA) * materialAlbedo.rgb;
    float metallic = saturate(metalRoughSample.b * materialMetallic);
    float roughness = clamp(metalRoughSample.g * materialRoughness, MIN_ROUGHNESS, MAX_ROUGHNESS);

    // 法線をワールド空間に変換する
    float3 N = normalize(pin.normal);
    float3 T = normalize(pin.tangent);
    float3 B = normalize(pin.bitangent);
    float3x3 TBN = float3x3(T, B, N);
    float3 worldNormal = normalize(mul(normalSample * NORMAL_MAP_SCALE - NORMAL_MAP_BIAS, TBN));

    // ディゾルブ処理
    if (hasDissolve > DISSOLVE_THRESHOLD_COMPARE)
    {
        // マスクを取得する
        float maskValue = dissolveMask.Sample(dissolveSampler, pin.texcoord).r;
    
        // 表示するか判定する
        float dissolveAlpha = step(dissolveThreshold, maskValue);
    
        // エッジ部分を計算する
        float edge = smoothstep(dissolveThreshold, dissolveThreshold + dissolveEdgeWidth, maskValue);
    
        // 発光色を決める
        float3 glowColor = (materialEmissiveStrength > ENEMY_EMISSIVE_THRESHOLD)
            ? EDGE_GLOW_COLOR_BLUE
            : EDGE_GLOW_COLOR_RED;
        
        // エッジ発光を作る
        float3 edgeColor = glowColor * dissolveEdgeIntensity * (1.0 - edge);
    
        // 発光に加える
        emissiveSample += edgeColor;
    
        // 完全に消えたら描画しない
        if (dissolveAlpha <= 0.0f)
            discard;
    }
    
    // 武器の発光処理
    float2 uvScrollSpeed = uvScrollSpeed_glowIntensity.xy;
    float glowIntensity = uvScrollSpeed_glowIntensity.z;
    float fresnelGlowPower = fresnelGlowPower_glowTime.x;
    float glowTime = fresnelGlowPower_glowTime.y;

    if (glowIntensity > 0.0f)
    {
        // UVをスクロールさせる
        float2 scrolledUV = pin.texcoord * GLOW_UV_SCALE;
        scrolledUV += glowTime * uvScrollSpeed * GLOW_UV_SPEED_FACTOR;
    
        // マスクを取得する
        float glowMask = emissiveMap.Sample(linearSampler, scrolledUV).r;
        glowMask = pow(glowMask, GLOW_MASK_POWER) * GLOW_MASK_SCALE;
    
        // フレネルを計算する
        float3 viewDir = normalize(cameraPosition.xyz - pin.worldPos);
        float fresnel = pow(1.0f - saturate(dot(N, viewDir)), fresnelGlowPower);
    
        // 発光を計算する
        float glow = (glowMask * GLOW_MULTIPLIER + fresnel * FRESNEL_GLOW_FACTOR) * glowIntensity;
        
        // 色を決める
        float3 glowColor = (uvScrollSpeed_glowIntensity.w < COLOR_THRESHOLD)
            ? GLOW_COLOR_RED
            : GLOW_COLOR_BLUE;
    
        emissiveSample += glow * glowColor;
    }
    
    // 視線とライト方向を計算する
    float3 V = normalize(cameraPosition.xyz - pin.worldPos);
    float3 L = normalize(-lightDirection.xyz);
    float3 H = normalize(L + V);

    // フレネル反射率を計算する
    float3 F0 = lerp(float3(DEFAULT_F0, DEFAULT_F0, DEFAULT_F0), baseColor, metallic);

    // 内積を計算する
    float NdotV = max(dot(worldNormal, V), NORMAL_DOT_MIN);
    float NdotL = max(dot(worldNormal, L), NORMAL_DOT_MIN);
    float NdotH = max(dot(worldNormal, H), NORMAL_DOT_MIN);
    float VdotH = max(dot(V, H), NORMAL_DOT_MIN);

    // BRDFを計算する
    float3 diffuseBRDF = DiffuseBRDF(VdotH, F0, baseColor) * lightColor.rgb * NdotL * DIFFUSE_BRIGHTNESS;
    float3 specularBRDF = SpecularBRDF(NdotV, NdotL, NdotH, VdotH, F0, roughness) * lightColor.rgb * NdotL;

    // シャドウを計算する
    float shadowFactor = 0.0f;
    
    for (int i = 0; i < SHADOW_SAMPLE_COUNT; ++i)
    {
        float2 offset = SHADOW_OFFSETS[i] * shadowTexelSize;
        shadowFactor += shadowMap.SampleCmpLevelZero(
            shadowSampler,
            pin.shadow.xy + offset,
            pin.shadow.z - SHADOW_BIAS
        ).r;
    }
    shadowFactor /= SHADOW_SAMPLE_COUNT;

    // ライティングを計算する
    float3 lighting = (diffuseBRDF + specularBRDF) * shadowFactor;

    // 環境光を加える
    lighting += baseColor * ao * AMBIENT_STRENGTH;

    // 環境反射を加える
    float3 specularIBL = IBL(worldNormal, V, roughness, F0);
    lighting += specularIBL * (SPECULAR_IBL_METALLIC_FACTOR * metallic);

    // シャドウの色を調整する
    float3 finalShadowColor = lerp(shadowColor.rgb, float3(1.0f, 1.0f, 1.0f), shadowFactor);
    lighting *= finalShadowColor;

    // 負の値を防ぐ
    lighting = max(lighting, 0.0f);

    // トーンマッピングを適用する
    float3 toneMapped = lighting / (lighting + 1.0f);

    // 発光を加える
    float3 colorOut = lighting + (emissiveSample * materialEmissiveColor.rgb)
                      * materialEmissiveStrength * EMISSIVE_STRENGTH_FACTOR;
    
    // ガンマ補正を行う
    colorOut = pow(max(colorOut, 0.0), 1.0 / GAMMA);
    
    return float4(colorOut, albedoSample.a);
}