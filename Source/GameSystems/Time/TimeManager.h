#pragma once
#include <DirectXMath.h>

class TimeManager {
public:
    static TimeManager& Instance() {
        static TimeManager instance;
        return instance;
    }

    // 毎フレーム更新
    void Update(float DeltaTime);

    // タイムスケール設定
    void SetTimeScale(float targetScale, float lerpSpeed = 5.0f); // 補間速度あり
    void ResetTimeScale(float lerpSpeed = 3.0f); // 通常速度に戻す
    void SetTimeScaleImmediate(float scale); // 即時変更

    // ヒットストップ
    void RequestHitStop(float duration, float newTimeScale = 0.01f); // 一時的に減速
    bool IsHitStopActive() const { return hitStopTimer > 0.0f; }

    // 取得
    float GetTimeScale() const { return timeScale; }           // 現在の速度
    float GetScaledDeltaTime() const { return scaledDeltaTime; } // スケール後Δ時間
    float GetRawDeltaTime() const { return rawDeltaTime; }     // 元のΔ時間
    bool IsTimeSlowed() const { return timeScale < TimeConstants::SLOW_THRESHOLD; } // 減速中か

private:
    TimeManager() = default;

    // タイムマネージャー定数
    struct TimeConstants
    {
        static constexpr float NORMAL_TIME_SCALE = 1.0f;
        static constexpr float DEFAULT_LERP_SPEED = 5.0f;
        static constexpr float RESET_LERP_SPEED = 3.0f;
        static constexpr float HITSTOP_TIME_SCALE = 0.01f;
        static constexpr float LERP_EPSILON = 0.001f;
        static constexpr float SLOW_THRESHOLD = 0.9f;
        static constexpr float RAPID_RESET_SPEED = 10.0f;
    };

    float timeScale = TimeConstants::NORMAL_TIME_SCALE;         // 現在の速度
    float targetTimeScale = TimeConstants::NORMAL_TIME_SCALE;   // 目標速度
    float timeScaleLerpSpeed = TimeConstants::DEFAULT_LERP_SPEED; // 補間速度

    float rawDeltaTime = 0.0f;      // 元のΔ時間
    float scaledDeltaTime = 0.0f;   // スケール後Δ時間

    // ヒットストップ用
    float hitStopTimer = 0.0f;      // 残り時間
    float hitStopTimeScale = TimeConstants::HITSTOP_TIME_SCALE; // 停止時の速度

    // 線形補間
    float Lerp(float a, float b, float t) {
        return a + t * (b - a);
    }
};