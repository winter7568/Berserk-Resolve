struct TrailVS_OUT
{
    float4 position : SV_Position; // 画面上の位置
    float2 texcoord : TEXCOORD;    // テクスチャ座標
    float3 worldPos : POSITION;    // ワールド座標
    float2 screenUV : TEXCOORD1;   // 画面用UV
};

struct TrailVS_IN
{
    float3 position : POSITION; // 頂点の位置
    float2 texcoord : TEXCOORD; // テクスチャ座標
};

cbuffer CbTrail : register(b2)
{
    float distortionStrength; // 歪みの強さ
    float trailAlpha;         // 透明度
    float2 uvScale;           // UVスケール 
    float time;               // 時間
    float isPlayer;           // プレイヤーかどうか
}

// トレイル頂点シェーダー定数
static const float POSITION_W = 1.0f;      // 頂点位置のW成分
static const float SCREEN_UV_SCALE = 0.5f; // 画面UVのスケール
static const float SCREEN_UV_BIAS = 1.0f;  // 画面UVのバイアス
static const float SCREEN_UV_Y_FLIP = 1.0f;// Y座標反転用

// トレイル定数
static const float CORE_REGION_START = 0.3f; // 中心領域の開始値
static const float CORE_REGION_END = 0.5f;   // 中心領域の終了値
static const float INNER_UV_SCALE_X = 3.0f;  // 内側UVスケールX
static const float INNER_UV_SCALE_Y = 2.0f;  // 内側UVスケールY
static const float INNER_UV_SPEED = 1.5f;    // 内側UVスクロール速度
static const float OUTER_UV_SCALE_X = 6.0f;  // 外側UVスケールX
static const float OUTER_UV_SCALE_Y = 3.0f;  // 外側UVスケールY
static const float OUTER_UV_SPEED = 2.5f;    // 外側UVスクロール速度
static const float NORMAL_SCALE = 2.0f;      // 法線のスケール（-1～1に変換）
static const float NORMAL_BIAS = 1.0f;       // 法線のバイアス
static const float COMBINED_NORMAL_SCALE = 2.0f; // 合成法線のスケール
static const float COMBINED_NORMAL_BIAS = 1.0f;  // 合成法線のバイアス
static const float DISTORTION_STRENGTH = 0.01f;  // 画面歪みの強さ
static const float EDGE_INTENSITY_FACTOR = 0.3f; // エッジ発光の強さ係数
static const float DEPTH_BLEND_EDGE = 0.15f;     // エッジ部分の深度ブレンド
static const float DEPTH_BLEND_CORE = 0.08f;     // 中心部分の深度ブレンド
static const float GLOW_INTENSITY_FACTOR = 0.4f; // 発光強度係数
static const float BASE_ALPHA_VOLUME = 0.8f; // 基本のボリュームアルファ
static const float EDGE_ALPHA_BOOST = 0.4f;  // エッジのアルファ増加量
static const float CORE_ALPHA_MIN = 0.7f;    // 中心のアルファ最小値
static const float CORE_ALPHA_MAX = 1.0f;    // 中心のアルファ最大値
static const float PULSE_SPEED = 3.0f;       // 脈動の速度
static const float PULSE_STRENGTH = 0.1f;    // 脈動の強さ
static const float PULSE_BASE = 0.9f;        // 脈動の基準値
static const float PLAYER_CHECK = 0.5f;      // プレイヤー判定用しきい値

// プレイヤーの色（赤系）
static const float3 PLAYER_CORE_COLOR = float3(0.8f, 0.15f, 0.05f);
static const float3 PLAYER_EDGE_COLOR = float3(1.5f, 0.6f, 0.2f);
static const float3 PLAYER_SURFACE_GLOW = float3(2.0f, 0.8f, 0.3f);

// 敵の色（青系）
static const float3 ENEMY_CORE_COLOR = float3(0.05f, 0.15f, 0.8f);
static const float3 ENEMY_EDGE_COLOR = float3(0.2f, 0.6f, 1.5f);
static const float3 ENEMY_SURFACE_GLOW = float3(0.3f, 0.8f, 2.0f);
