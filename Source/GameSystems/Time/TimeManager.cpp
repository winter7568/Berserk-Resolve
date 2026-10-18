#include <cmath>

#include "TimeManager.h"

void TimeManager::Update(float deltaTime)
{
    rawDeltaTime = deltaTime; // 元時間保存

    // ヒットストップ優先
    if (hitStopTimer > 0.0f) {
        hitStopTimer -= deltaTime; // 実時間で減少

        if (hitStopTimer <= 0.0f) {
            // 終了
            hitStopTimer = 0.0f;
            SetTimeScale(TimeConstants::NORMAL_TIME_SCALE, TimeConstants::RAPID_RESET_SPEED); // 早く復帰
        }
        else {
            // 停止中は固定
            timeScale = hitStopTimeScale;
            targetTimeScale = hitStopTimeScale;
            scaledDeltaTime = deltaTime * timeScale;
            return; // 通常処理しない
        }
    }

    // 通常補間
    if (timeScale != targetTimeScale) {
        timeScale = Lerp(timeScale, targetTimeScale, timeScaleLerpSpeed * deltaTime);

        // 誤差対策
        if (std::fabs(timeScale - targetTimeScale) < TimeConstants::LERP_EPSILON) {
            timeScale = targetTimeScale;
        }
    }

    // スケール後時間
    scaledDeltaTime = deltaTime * timeScale;
}

void TimeManager::SetTimeScale(float targetScale, float lerpSpeed) {
    targetTimeScale = targetScale;     // 目標スケール
    timeScaleLerpSpeed = lerpSpeed;    // 補間速度
}

void TimeManager::ResetTimeScale(float lerpSpeed) {
    SetTimeScale(TimeConstants::NORMAL_TIME_SCALE, lerpSpeed); // 1.0が通常速度
}

void TimeManager::SetTimeScaleImmediate(float scale) {
    timeScale = scale;         // 現在値
    targetTimeScale = scale;   // 目標も同じにする
}

void TimeManager::RequestHitStop(float duration, float newTimeScale) {
    hitStopTimer = duration;          // 継続時間
    hitStopTimeScale = newTimeScale;  // 停止中のスケール
    timeScale = newTimeScale;         // 即時反映
    targetTimeScale = newTimeScale;   // 補間なしで固定
}