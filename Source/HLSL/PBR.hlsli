#ifndef PBR_HLSLI
#define PBR_HLSLI

struct PBR_VS_OUT
{
    float4 position : SV_Position; // 画面上の位置
    float2 texcoord : TEXCOORD;    // テクスチャ座標
    float3 normal : NORMAL;        // 法線ベクトル
    float3 worldPos : POSITION;    // ワールド座標
    float3 tangent : TANGENT;      // タンジェント
    float3 bitangent : BITANGENT;  // ビタンジェント
    float3 shadow : SHADOW;        // シャドウ用の座標
};

cbuffer CbScene : register(b0)
{
    row_major float4x4 viewProjection;      // ビューと投影を合わせた行列
    float4 lightDirection;                  // ライトの方向
    float4 lightColor;                      // ライトの色
    float4 cameraPosition;                  // カメラの位置
    row_major float4x4 lightViewProjection; // シャドウ用のビューと投影行列
    float4 shadowColor;                     // シャドウの色
    float shadowTexelSize;                  // シャドウマップの1ピクセルの大きさ
    float pad[3];                           // 調整用の空き
}

cbuffer CbMaterial : register(b1)
{
    float4 materialAlbedo;          // ベースカラー
    float4 materialEmissiveColor;   // 発光色
    float materialMetallic;         // メタリック値
    float materialRoughness;        // ラフネス
    float materialAO;               // 環境光オクルージョン
    float materialEmissiveStrength; // 発光の強さ
    
    float dissolveThreshold;     // ディゾルブのしきい値
    float dissolveEdgeWidth;     // エッジの幅
    float dissolveEdgeIntensity; // エッジの強さ
    float hasDissolve;           // ディゾルブを使うかどうか

    float materialEnvironmentStrength; // 環境反射の強さ
    float materialPadding[3];          // 調整用の空き
}

cbuffer CbWeaponGlow : register(b3)
{
    float4 uvScrollSpeed_glowIntensity; // UVスクロール速度と発光の強さ
    float4 fresnelGlowPower_glowTime;   // フレネルの強さと時間
};

// PBR定数
static const int SHADOW_SAMPLE_COUNT = 9;               // シャドウサンプル数
static const float SHADOW_BIAS = 0.001f;                // シャドウバイアス
static const float HALF_SIZE = 0.5;                       // シャドウサンプル数
static const float GAMMA = 2.2f;                        // ガンマ値
static const float MIN_ROUGHNESS = 0.05f;               // ラフネスの最小値
static const float MAX_ROUGHNESS = 1.0f;                // ラフネスの最大値
static const float NORMAL_MAP_SCALE = 2.0f;             // 法線マップのスケール（-1～1に変換）
static const float NORMAL_MAP_BIAS = 1.0f;              // 法線マップのバイアス
static const float DISSOLVE_THRESHOLD_COMPARE = 0.5f;   // ディゾルブ比較用しきい値
static const float MAX_ENV_COLOR = 20.0f;               // 環境色の最大値
static const float DEFAULT_F0 = 0.04f;                  // デフォルトのフレネル反射率
static const float NORMAL_DOT_MIN = 0.0001f;            // 内積の最小値（ゼロ除算防止）
static const float DIFFUSE_BRIGHTNESS = 1.5f;           // 拡散反射の明るさ倍率
static const float AMBIENT_STRENGTH = 0.5f;             // 環境光の強さ
static const float SPECULAR_IBL_METALLIC_FACTOR = 0.2f; // スペキュラIBLのメタリック係数
static const float EMISSIVE_STRENGTH_FACTOR = 0.5f;     // 発光強度の係数

// 武器発光定数
static const float COLOR_THRESHOLD = 0.5f;           // 敵とプレイヤーの色判断定数
static const float ENEMY_EMISSIVE_THRESHOLD = 30.0f; // 敵と味方の境界値
static const float GLOW_UV_SCALE = 8.0f;             // 発光UVのスケール
static const float GLOW_UV_SPEED_FACTOR = 0.2f;      // 発光UVスクロール速度係数
static const float GLOW_MASK_POWER = 2.0f;           // 発光マスクのべき乗
static const float GLOW_MASK_SCALE = 40.0f;          // 発光マスクのスケール
static const float GLOW_MULTIPLIER = 3.0f;           // 発光の倍率
static const float FRESNEL_GLOW_FACTOR = 0.8f;       // フレネル発光係数

static const float3 GLOW_COLOR_RED = float3(0.9f, 0.0f, 0.0f);        // 発光色（赤）
static const float3 GLOW_COLOR_BLUE = float3(0.0f, 0.4f, 1.0f);       // 発光色（青）
static const float3 EDGE_GLOW_COLOR_BLUE = float3(0.0f, 0.01f, 1.0f); // エッジ発光色（青）
static const float3 EDGE_GLOW_COLOR_RED = float3(1.0f, 0.01f, 0.0f);  // エッジ発光色（赤）

// シャドウオフセット
static const float2 SHADOW_OFFSETS[9] =
{
    float2(-1, -1), float2(0, -1), float2(1, -1),
    float2(-1, 0), float2(0, 0), float2(1, 0),
    float2(-1, 1), float2(0, 1), float2(1, 1)
};


#endif // PBR_HLSLI