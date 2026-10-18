#pragma once
#include <DirectXMath.h>

#include "Camera.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/Enemy/Enemy.h"

class LockOnController
{
public:
    // シングルトン取得
    static LockOnController& Instance()
    {
        static LockOnController instance;
        return instance;
    }

    // 毎フレーム更新
    void Update(float elapsedTime);

    // ロックオンの切り替え
    void ToggleLockOn();
    void SetLockOnEnabled(bool enabled);

    // ロックオン状態を取得
    bool IsLockedOn() const { return isLockedOn; }

    // ターゲット設定
    void SetTarget(Character* target);

    // 現在のターゲット取得
    Character* GetTarget() const { return currentTarget; }

    // カメラ補間の滑らかさ設定
    void SetSmoothness(float smooth) { smoothness = smooth; }

    // 現在の滑らかさ取得
    float GetSmoothness() const { return smoothness; }

private:
    // コンストラクタ（外部から生成不可）
    LockOnController() = default;

    bool isLockedOn = false;           // ロックオン中かどうか
    Character* currentTarget = nullptr; // 現在のターゲット

    // カメラ調整用パラメータ
    float baseDistance = 25.0f;        // 基本距離
    float baseHeight = 14.0f;          // 基本高さ
    float maxDistanceScale = 1.5f;     // 距離の最大倍率
    float maxHeightScale = 1.3f;       // 高さの最大倍率
    float distanceScaleFactor = 100.0f;// 距離変化の影響度
    float heightScaleFactor = 150.0f;  // 高さ変化の影響度
    float verticalFocusOffset = 8.0f;  // 注視点の高さオフセット
    float smoothness = 20.0f;          // 補間の速さ

    // カメラ位置（補間用）
    DirectX::XMFLOAT3 currentCameraPosition = { 0,0,0 };

    // 定数
    static constexpr float HORIZONTAL_DISTANCE_EPSILON = 0.001f; // 0除算防止
    static constexpr float FOCUS_POINT_MIDPOINT = 0.5f;          // 中間点割合
    static constexpr float ARENA_BOUNDARY_MARGIN = 10.0f;        // 境界マージン
    static constexpr float ARENA_DIST_EPSILON = 0.0001f;         // 距離チェック用
    static constexpr float SHAKE_FOCUS_FACTOR = 0.3f;            // シェイク影響度
};