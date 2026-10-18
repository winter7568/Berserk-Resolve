#include <DirectXMath.h>
#include <algorithm>

#include "GameLib/Graphics.h"
#include "GameLib/Input.h"
#include "CameraController.h"
#include "Camera.h"
#include "LockOnController.h"
#include "CameraShake.h" 
#include "GameObjects/Enemy/Enemy.h" 
#include "GameObjects/Arena/Arena.h" 

void CameraController::Update(float elapsedTime)
{
    // カメラシェイク更新
    CameraShake::Instance().Update(elapsedTime);

    // バーサークモード更新
    UpdateBerserkMode(elapsedTime);

    // ロックオン中はそちらを使用
    if (LockOnController::Instance().IsLockedOn()) {
        LockOnController::Instance().Update(elapsedTime);
        return;
    }

    // 入力処理
    ProcessInput(elapsedTime);

    // カメラ位置計算
    DirectX::XMFLOAT3 eye = CalculateCameraPosition();

    // 境界制限
    ClampCameraToBoundary(eye);

    // ゲートキューブ衝突回避
    ClampCameraToGateCube(eye);

    // カメラに反映
    ApplyCameraTransform(eye);
}

void CameraController::ResetToBehindPlayer(const DirectX::XMFLOAT3& playerPos, float playerAngleY)
{
    // playerAngleY はプレイヤーのY軸回転（向き）
    angle.y = playerAngleY;

    // カメラの距離と高さを初期値に戻す
    currentCameraDistance = DEFAULT_CAMERA_DISTANCE;
    currentCameraHeight = DEFAULT_CAMERA_HEIGHT;

    // カメラの注視点をプレイヤー位置に設定
    target = playerPos;
}

void CameraController::UpdateBerserkMode(float elapsedTime)
{
    // 敵の状態チェック
    bool BerserkMode = Enemy::Instance().GetStateManager().IsInState(EnemyStateID::BERSERK);
    bool PowerChargeMode = Enemy::Instance().GetStateManager().IsInState(EnemyStateID::POWER_CHARGE);

    // バーサーク用カメラを使うか
    bool shouldUseBerserkCamera = BerserkMode || PowerChargeMode;

    // モード切り替え
    if (shouldUseBerserkCamera != isBerserkMode) {
        isBerserkMode = shouldUseBerserkCamera;
    }

    // 目標距離・高さ
    float targetDistance = isBerserkMode ? berserkCameraDistance : DEFAULT_CAMERA_DISTANCE;
    float targetHeight = isBerserkMode ? berserkCameraHeight : DEFAULT_CAMERA_HEIGHT;

    // スムーズ補間
    float transitionSpeed = CAMERA_TRANSITION_SPEED;
    currentCameraDistance += (targetDistance - currentCameraDistance) * transitionSpeed * elapsedTime;
    currentCameraHeight += (targetHeight - currentCameraHeight) * transitionSpeed * elapsedTime;
}

void CameraController::ProcessInput(float elapsedTime)
{
    // 入力取得
    Mouse& mouse = Input::Instance().GetMouse();
    auto& gamePad = Input::Instance().GetGamePad();

    // マウス移動量
    int deltaX = mouse.GetPositionX() - mouse.GetOldPositionX();
    int deltaY = mouse.GetPositionY() - mouse.GetOldPositionY();

    // 右スティック入力
    float rawStickX = gamePad.GetAxisRX();
    float rawStickY = gamePad.GetAxisRY();

    // 反転設定
    float stickX = invertJoystickX ? -rawStickX : rawStickX;
    float stickY = invertJoystickY ? -rawStickY : rawStickY;

    // デッドゾーン処理
    float stickMagnitude = sqrtf(stickX * stickX + stickY * stickY);
    if (stickMagnitude < joystickDeadzone) {
        stickX = 0.0f;
        stickY = 0.0f;
    }
    else {
        float normalizedMagnitude = (stickMagnitude - joystickDeadzone) / (1.0f - joystickDeadzone);
        stickX = (stickX / stickMagnitude) * normalizedMagnitude;
        stickY = (stickY / stickMagnitude) * normalizedMagnitude;
    }

    // スムージング
    smoothedStickX = smoothedStickX * (1.0f - smoothFactor) + stickX * smoothFactor;
    smoothedStickY = smoothedStickY * (1.0f - smoothFactor) + stickY * smoothFactor;

    // マウス＋スティック合成
    float totalDeltaX = static_cast<float>(deltaX) + (stickX * joystickSensitivity);
    float totalDeltaY = static_cast<float>(deltaY) + (stickY * joystickSensitivity);

    // 回転入力
    float speed = mouseSensitivity * elapsedTime;
    float yawInput = totalDeltaX * speed;
    float pitchInput = totalDeltaY * speed;

    // 角度更新
    angle.y += yawInput;
    angle.x += pitchInput;

    // ピッチ制限
    const float maxPitch = DirectX::XM_PIDIV4 - MAX_PITCH_OFFSET;
    const float minPitch = 0.0f;
    angle.x = std::clamp(angle.x, minPitch, maxPitch);

    // ヨーを範囲内に収める
    if (angle.y < -DirectX::XM_PI) angle.y += DirectX::XM_2PI;
    if (angle.y > DirectX::XM_PI) angle.y -= DirectX::XM_2PI;
}

DirectX::XMFLOAT3 CameraController::CalculateCameraPosition()
{
    // 回転行列作成
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, 0);

    // 前方向ベクトル
    DirectX::XMVECTOR camDirection = rotationMatrix.r[2];
    camDirection = DirectX::XMVector3Normalize(camDirection);

    // 距離と高さ
    float distanceBack = currentCameraDistance;
    float heightAbove = currentCameraHeight;

    // ターゲット位置（少し上）
    DirectX::XMVECTOR targetVec = XMLoadFloat3(&target);
    targetVec = DirectX::XMVectorSetY(targetVec, DirectX::XMVectorGetY(targetVec) + 1.0f);

    // カメラ位置計算（後ろ＋上）
    DirectX::XMVECTOR offset = DirectX::XMVectorAdd(
         DirectX::XMVectorScale(DirectX::XMVectorNegate(camDirection), distanceBack),
         DirectX::XMVectorSet(0, heightAbove, 0, 0)
    );
    DirectX::XMVECTOR eyeVec = DirectX::XMVectorAdd(targetVec, offset);

    // シェイク適用
    if (CameraShake::Instance().IsShaking()) {
        auto shakeOffset = CameraShake::Instance().GetShakeOffset();
        DirectX::XMVECTOR shakeVec = XMLoadFloat3(&shakeOffset);
        eyeVec = DirectX::XMVectorAdd(eyeVec, shakeVec);
    }

    DirectX::XMFLOAT3 eye;
    DirectX::XMStoreFloat3(&eye, eyeVec);
    return eye;
}

void CameraController::ClampCameraToBoundary(DirectX::XMFLOAT3& eye) const
{
    // パス制限（ボックス）
    if (Arena::Instance().UsingPathBoundary())
    {
        DirectX::XMFLOAT3 pathCenter = Arena::Instance().GetPathCenter();
        DirectX::XMFLOAT3 pathSize = Arena::Instance().GetPathSize();

        float minX = pathCenter.x - pathSize.x - CAM_MARGIN;
        float maxX = pathCenter.x + pathSize.x + CAM_MARGIN;
        float minZ = pathCenter.z - pathSize.z - CAM_MARGIN;
        float maxZ = pathCenter.z + pathSize.z + CAM_MARGIN;

        // 範囲外なら補正
        if (eye.x < minX) eye.x = minX;
        else if (eye.x > maxX) eye.x = maxX;

        if (eye.z < minZ) eye.z = minZ;
        else if (eye.z > maxZ) eye.z = maxZ;
    }
    else
    {
        // 円形制限
        DirectX::XMFLOAT3 center = Arena::Instance().GetBoundaryCenter();
        float radius = Arena::Instance().GetBoundaryRadius() + CAM_MARGIN;

        float dx = eye.x - center.x;
        float dz = eye.z - center.z;
        float dist = sqrtf(dx * dx + dz * dz);

        // 外に出たら戻す
        if (dist > radius && dist > 0.0001f)
        {
            dx /= dist;
            dz /= dist;
            eye.x = center.x + dx * radius;
            eye.z = center.z + dz * radius;
        }
    }
}

void CameraController::ClampCameraToGateCube(DirectX::XMFLOAT3& eye) const
{
    // ゲートキューブ衝突（侵入禁止）
    auto& arena = Arena::Instance();
    if (!arena.IsGateCubeVisible() || !arena.IsGateCubeSolid()) return;

    DirectX::XMFLOAT3 cubeCenter = arena.GetGateCubeCenter();
    DirectX::XMFLOAT3 cubeSize = arena.GetGateCubeSize();

    float minX = cubeCenter.x - cubeSize.x;
    float maxX = cubeCenter.x + cubeSize.x;
    float minY = cubeCenter.y - cubeSize.y;
    float maxY = cubeCenter.y + cubeSize.y;
    float minZ = cubeCenter.z - cubeSize.z;
    float maxZ = cubeCenter.z + cubeSize.z;

    // 中に入るかチェック
    bool wouldBeInside = (eye.x >= minX && eye.x <= maxX &&
                          eye.y >= minY && eye.y <= maxY &&
                          eye.z >= minZ && eye.z <= maxZ);

    if (!wouldBeInside) return;

    // 一番近い面に押し出す
    float distToMinX = abs(eye.x - minX);
    float distToMaxX = abs(eye.x - maxX);
    float distToMinY = abs(eye.y - minY);
    float distToMaxY = abs(eye.y - maxY);
    float distToMinZ = abs(eye.z - minZ);
    float distToMaxZ = abs(eye.z - maxZ);

    float closest = min(min(min(min(min(distToMinX, distToMaxX), distToMinY), distToMaxY), distToMinZ), distToMaxZ);

    if (closest == distToMinX) eye.x = minX - GATE_CUBE_PUSH_OFFSET;
    else if (closest == distToMaxX) eye.x = maxX + GATE_CUBE_PUSH_OFFSET;
    else if (closest == distToMinY) eye.y = minY - GATE_CUBE_PUSH_OFFSET;
    else if (closest == distToMaxY) eye.y = maxY + GATE_CUBE_PUSH_OFFSET;
    else if (closest == distToMinZ) eye.z = minZ - GATE_CUBE_PUSH_OFFSET;
    else if (closest == distToMaxZ) eye.z = maxZ + GATE_CUBE_PUSH_OFFSET;
}

void CameraController::ApplyCameraTransform(const DirectX::XMFLOAT3& eye)
{
    DirectX::XMFLOAT3 targetPos;
    DirectX::XMVECTOR targetVec = XMLoadFloat3(&target);
    targetVec = DirectX::XMVectorSetY(targetVec, DirectX::XMVectorGetY(targetVec) + 1.0f);
    DirectX::XMStoreFloat3(&targetPos, targetVec);

    // カメラに反映
    Camera::Instance().SetLookAt(eye, targetPos, DirectX::XMFLOAT3(0, 1, 0));

    // マウスを中央に戻す
    Mouse& mouse = Input::Instance().GetMouse();
    mouse.CenterCursor();
}