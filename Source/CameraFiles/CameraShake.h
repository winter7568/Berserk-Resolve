#pragma once
#include <DirectXMath.h>
#include <cstdio>       
#include <Windows.h>

// カメラシェイク管理クラス
class CameraShake {
public:
    // シングルトン取得
    static CameraShake& Instance() {
        static CameraShake instance;
        return instance;
    }

    // トラウマ（揺れ強さ）を追加
    void AddTrauma(float trauma) {
        currentTrauma += trauma;
        if (currentTrauma > MAX_TRAUMA) currentTrauma = MAX_TRAUMA; // 最大値で制限
    }

    // 毎フレーム更新
    void Update(float elapsedTime);

    // 現在の揺れオフセットを取得
    DirectX::XMFLOAT3 GetShakeOffset() const { return shakeOffset; }

    // 揺れているか判定
    bool IsShaking() const { return currentTrauma > SHAKE_THRESHOLD; }

private:
    CameraShake() = default; // 外部から生成不可

    float currentTrauma = 0.0f; // 現在の揺れ強さ
    float traumaDecay = 3.5f;   // 減衰速度（大きいほど早く消える）
    float shakeFrequency = 50.0f; // 揺れの速さ
    float maxShake = 3.5f;        // 最大揺れ量

    DirectX::XMFLOAT3 shakeOffset = { 0.0f, 0.0f, 0.0f }; // 現在のオフセット
    float shakeTime = 0.0f; // 時間（ノイズ用）

    // ノイズ生成関数（滑らかなランダム）
    float Noise1D(float time, float frequency) const;
    float Noise2D(float time, float frequency) const;

    // 定数
    static constexpr float MAX_TRAUMA = 2.0f;          // 最大トラウマ
    static constexpr float SHAKE_THRESHOLD = 0.01f;    // 揺れ判定の閾値
    static constexpr float DEBUG_INTERVAL = 0.5f;      // デバッグ用（未使用）
    static constexpr float NOISE_TIME_OFFSET = 100.0f; // ノイズ時間ずらし

    // ノイズ用定数
    static constexpr float NOISE_HARMONIC1_AMP = 0.5f;       // 1番目の倍音の強さ
    static constexpr float NOISE_HARMONIC2_AMP = 0.25f;      // 2番目の倍音の強さ
    static constexpr float NOISE_HARMONIC3_AMP = 0.125f;     // 3番目の倍音の強さ
    static constexpr float NOISE_HARMONIC2_FREQ = 2.1f;      // 2番目の倍音の周波数
    static constexpr float NOISE_HARMONIC3_FREQ = 4.3f;      // 3番目の倍音の周波数
    static constexpr float NOISE_2D_PRIMARY_WEIGHT = 0.7f;   // 2Dノイズの主成分の重み
    static constexpr float NOISE_2D_SECONDARY_WEIGHT = 0.3f; // 2Dノイズの副成分の重み
    static constexpr float NOISE_2D_TIME_OFFSET = 50.0f;     // 2Dノイズの時間オフセット
    static constexpr float NOISE_2D_FREQ_SCALE = 1.3f;       // 2Dノイズの周波数スケール
};