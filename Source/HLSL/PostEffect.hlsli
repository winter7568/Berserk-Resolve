cbuffer CbPostEffect : register(b0)
{
    float luminanceExtractionLowerEdge;  // 明るさ抽出の下限値
    float luminanceExtractionHigherEdge; // 明るさ抽出の上限値
    
    float gaussianSigma;  // ガウシアンブラーの強さ
    float bloomIntensity; // ブルームの強さ
    
    float vignetteIntensity; // ビネットの強さ
    float vignetteScale;     // ビネットの範囲
    float vignetteEnabled;   // ビネットを使うかどうか
    float screenFadeAlpha;   // 画面フェードの強さ
};


struct TrailVS_OUT
{
    float4 position : SV_Position; // 画面上の位置
    float2 texcoord : TEXCOORD;    // テクスチャ座標
    float3 worldPos : POSITION;    // ワールド座標
    float2 screenUV : TEXCOORD1;   // 画面用UV
};

cbuffer CbTrail : register(b2)
{
    float distortionStrength; // 歪みの強さ
    float trailAlpha;         // 透明度
    float2 uvScale;           // UVスケール
    float time;               // 時間
    float isPlayer;           // プレイヤーかどうか
}

// ブルーム定数
static const float LUMINANCE_R = 0.299f;
static const float LUMINANCE_G = 0.587f;
static const float LUMINANCE_B = 0.114f;
static const float EMISSIVE_THRESHOLD = 0.3f;
static const float TAU = 6.283185307179586f; // 2 * PI
static const float PI = 3.14159265358979f;
static const int HALF_KERNEL_SIZE = 3;
static const float EXPOSURE = 1.2f;

// ビネット定数
static const float VIGNETTE_UV_CENTER = 0.5f;      // UVの中心値
static const float VIGNETTE_UV_BIAS = 0.5f;        // UVのバイアス
static const float DEFAULT_MASK = 1.0f;            // デフォルトのマスク値
static const float VIGNETTE_COLOR_STRENGTH = 0.3f; // ビネット色の強さ
static const float VIGNETTE_ENABLE_CHECK = 0.5f;   // ビネット有効判定用しきい値
static const float3 VIGNETTE_COLOR = float3(0.1f, 0.05f, 0.2f);// ビネットの色（暗い紫）
