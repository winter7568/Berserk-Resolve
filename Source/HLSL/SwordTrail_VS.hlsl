#include "PBR.hlsli"
#include "SwordTrail.hlsli"

TrailVS_OUT main(TrailVS_IN input)
{
    TrailVS_OUT output;
    
    // ワールド座標をクリップ空間に変換する
    float4 worldPos = float4(input.position, POSITION_W);
    output.position = mul(worldPos, viewProjection);
    output.worldPos = input.position;
    output.texcoord = input.texcoord;
    
    // 画面用のUVを計算する
    output.screenUV.x = (output.position.x / output.position.w + SCREEN_UV_BIAS) * SCREEN_UV_SCALE;
    output.screenUV.y = SCREEN_UV_Y_FLIP - (output.position.y / output.position.w + SCREEN_UV_BIAS) * SCREEN_UV_SCALE;
        
    return output;
}