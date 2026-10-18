#include <cmath>
#include "CameraFiles/CameraShake.h"

// カメラシェイクの更新処理
void CameraShake::Update(float elapsedTime) 
{
    // トラウマ（揺れ強さ）が0なら揺れなし
    if (currentTrauma <= 0.0f) {
        shakeOffset = { 0.0f, 0.0f, 0.0f };
        return;
    }

    // 時間を加算（ノイズ用）
    shakeTime += elapsedTime;

    // 揺れの強さ（2乗して強調）
    float intensity = currentTrauma * currentTrauma;

    // ノイズで揺れを生成（X方向）
    shakeOffset.x = Noise2D(shakeTime, shakeFrequency) * intensity * maxShake;

    // Y方向（時間をずらして違う揺れにする）
    shakeOffset.y = Noise2D(shakeTime + NOISE_TIME_OFFSET, shakeFrequency) * intensity * maxShake;

    // Z方向は通常揺らさない
    shakeOffset.z = 0.0f;

    // トラウマを時間で減衰させる
    currentTrauma -= traumaDecay * elapsedTime;
    if (currentTrauma < 0.0f) currentTrauma = 0.0f;
}

// 1次元ノイズ（sin波で簡易ランダム）
float CameraShake::Noise1D(float time, float frequency) const 
{
    return sinf(time * frequency) * NOISE_HARMONIC1_AMP +                      // 基本波
        sinf(time * frequency * NOISE_HARMONIC2_FREQ) * NOISE_HARMONIC2_AMP + // 2つ目の波
        sinf(time * frequency * NOISE_HARMONIC3_FREQ) * NOISE_HARMONIC3_AMP;  // 3つ目の波
}

// 2次元風ノイズ（複数のノイズを合成）
float CameraShake::Noise2D(float time, float frequency) const 
{
    return Noise1D(time, frequency) * NOISE_2D_PRIMARY_WEIGHT + // メインノイズ
        Noise1D(time + NOISE_2D_TIME_OFFSET, frequency * NOISE_2D_FREQ_SCALE) * NOISE_2D_SECONDARY_WEIGHT; // 補助ノイズ
}