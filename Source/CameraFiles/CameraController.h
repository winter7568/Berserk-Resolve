#pragma once
#include <DirectXMath.h>
#include "GameObjects/Arena/Arena.h"

class CameraController
{
public:
    void Update(float elapsedTime); 

    void SetTarget(const DirectX::XMFLOAT3& target) { this->target = target; } // 注視点（プレイヤー位置など）を設定

    // 感度設定
    void SetMouseSensitivity(float sensitivity) { mouseSensitivity = sensitivity; }       // マウス感度
    void SetJoystickSensitivity(float sensitivity) { joystickSensitivity = sensitivity; } // スティック感度
    void SetJoystickDeadzone(float deadzone) { joystickDeadzone = deadzone; }             // デッドゾーン
    void SetJoystickInvertX(bool invert) { invertJoystickX = invert; }                    // X反転
    void SetJoystickInvertY(bool invert) { invertJoystickY = invert; }                    // Y反転

    float GetMouseSensitivity() const { return mouseSensitivity; }       // マウス感度取得
    float GetJoystickSensitivity() const { return joystickSensitivity; } // スティック感度取得

    void ResetToBehindPlayer(const DirectX::XMFLOAT3& playerPos, float playerAngleY); // プレイヤーの後ろにリセット

private:
    // ヘルパー関数
    void UpdateBerserkMode(float elapsedTime);
    void ProcessInput(float elapsedTime);
    DirectX::XMFLOAT3 CalculateCameraPosition();
    void ClampCameraToBoundary(DirectX::XMFLOAT3& eye)const;
    void ClampCameraToGateCube(DirectX::XMFLOAT3& eye)const;
    void ApplyCameraTransform(const DirectX::XMFLOAT3& eye);

    DirectX::XMFLOAT3 target = { 0,0,0 }; // カメラの注視点
    DirectX::XMFLOAT3 angle = { 0,0,0 };  // カメラの角度（ピッチ・ヨー）
    float range = 3.0f;                   // 未使用 or 予備
    float maxAngleX = DirectX::XMConvertToRadians(45);  // 上方向の最大角度
    float minAngleX = DirectX::XMConvertToRadians(-45); // 下方向の最小角度

    // カメラ操作設定
    float mouseSensitivity = 0.8f;       // マウス感度
    float joystickSensitivity = 10.0f;   // スティック感度
    float joystickDeadzone = 0.15f;      // デッドゾーン
    bool invertJoystickX = false;        // X反転
    bool invertJoystickY = false;        // Y反転

    // スティック入力のスムージング
    float smoothedStickX = 0.0f;
    float smoothedStickY = 0.0f;
    float smoothFactor = 0.2f; // 補間の強さ

    DirectX::XMFLOAT3 lastValidCameraPos = { 0, 0, 0 }; // 最後の有効なカメラ位置

    // デバッグ用
    bool showCameraDebug = false;
    DirectX::XMFLOAT3 lastRayOrigin = {};        // レイ開始位置
    DirectX::XMFLOAT3 lastRayDirection = {};     // レイ方向
    DirectX::XMFLOAT3 lastIntersectionPoint = {};// 衝突位置
    bool lastHadIntersection = false;            // 衝突したか

    float berserkCameraDistance = 60.0f; // バーサーク時の距離
    float berserkCameraHeight = 7.0f;    // バーサーク時の高さ
    float currentCameraDistance = 25.0f; // 現在の距離（補間用）
    float currentCameraHeight = 2.5f;    // 現在の高さ（補間用）
    bool isBerserkMode = false;          // バーサーク状態かどうか

    // デフォルト値
    static constexpr float DEFAULT_CAMERA_DISTANCE = 35.0f;
    static constexpr float DEFAULT_CAMERA_HEIGHT = 4.5f;
    static constexpr float CAMERA_TRANSITION_SPEED = 3.0f; // 補間スピード
    static constexpr float CAM_MARGIN = 5.0f;              // 境界マージン
    static constexpr float MAX_PITCH_OFFSET = 0.01f;       // ピッチ制限オフセット
    static constexpr float GATE_CUBE_PUSH_OFFSET = 1.0f;   // ゲート押し出しオフセット
};