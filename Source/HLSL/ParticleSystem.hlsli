struct VS_IN
{
    float3 position : POSITION; // 頂点の位置
    float2 size : TEXCOORD;     // サイズ 横幅と高さ
    float4 color : COLOR;       // 色
    float4 param : PARAMETER;   // 回転や種類などの追加情報
};

struct GS_IN
{
    float3 position : POSITION; // 頂点の位置
    float2 size : TEXCOORD;     // サイズ 横幅と高さ
    float4 color : COLOR;       // 色
    float4 param : PARAMETER;   // 回転や種類などの追加情報
};

struct PS_IN
{
    float4 position : SV_POSITION; // 画面上の位置
    float4 color : COLOR;          // 色
    float2 texcoord : TEXCOORD;    // テクスチャ座標
};

cbuffer CbScene : register(b0)
{
    row_major float4x4 view;       // ビュー行列
    row_major float4x4 projection; // 投影行列
};