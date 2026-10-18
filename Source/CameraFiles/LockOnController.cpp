#include <DirectXMath.h>
#include <algorithm>

#include "LockOnController.h"
#include "CameraController.h"
#include "CameraShake.h" 
#include "GameObjects/Arena/Arena.h"

void LockOnController::Update(float elapsedTime)
{
    // カメラシェイク更新
    CameraShake::Instance().Update(elapsedTime);

    // ロックオンしていない、またはターゲットなしなら終了
    if (!isLockedOn || !currentTarget) return;

    // プレイヤーと敵の位置取得
    Player& player = Player::Instance();
    DirectX::XMFLOAT3 playerPos = player.GetPosition();
    DirectX::XMFLOAT3 targetPos = currentTarget->GetPosition();

    // プレイヤー→敵の方向（XZ平面のみ）
    DirectX::XMFLOAT3 dirToEnemy = {
        targetPos.x - playerPos.x,
        0.0f,
        targetPos.z - playerPos.z
    };

    float horizontalDistance = sqrtf(dirToEnemy.x * dirToEnemy.x + dirToEnemy.z * dirToEnemy.z);
    if (horizontalDistance > HORIZONTAL_DISTANCE_EPSILON) {
        dirToEnemy.x /= horizontalDistance;
        dirToEnemy.z /= horizontalDistance;
    }

    // プレイヤーと敵の3D距離
    float fullSeparation = sqrtf(
        (targetPos.x - playerPos.x) * (targetPos.x - playerPos.x) +
        (targetPos.y - playerPos.y) * (targetPos.y - playerPos.y) +
        (targetPos.z - playerPos.z) * (targetPos.z - playerPos.z)
    );

    // 距離に応じてカメラ距離と高さを調整
    float distanceScale = min(maxDistanceScale, 1.0f + (fullSeparation / distanceScaleFactor));
    float heightScale = min(maxHeightScale, 1.0f + (fullSeparation / heightScaleFactor));

    float dynamicDistance = baseDistance * distanceScale;
    float dynamicHeight = baseHeight * heightScale;

    // カメラ位置（プレイヤー基準）
    DirectX::XMFLOAT3 desiredCameraPos = playerPos;

    // 敵の反対方向へ移動
    desiredCameraPos.x -= dirToEnemy.x * dynamicDistance;
    desiredCameraPos.z -= dirToEnemy.z * dynamicDistance;

    // 上方向へ移動
    desiredCameraPos.y += dynamicHeight;

    // アリーナ内に制限
    if (Arena::Instance().PlayerHasEntered())
    {
        DirectX::XMFLOAT3 center = Arena::Instance().GetBoundaryCenter();
        float radius = Arena::Instance().GetBoundaryRadius() + ARENA_BOUNDARY_MARGIN;

        float dx = desiredCameraPos.x - center.x;
        float dz = desiredCameraPos.z - center.z;

        float dist = sqrtf(dx * dx + dz * dz);

        if (dist > radius && dist > ARENA_DIST_EPSILON)
        {
            dx /= dist;
            dz /= dist;

            desiredCameraPos.x = center.x + dx * radius;
            desiredCameraPos.z = center.z + dz * radius;
        }
    }

    // 注視点（プレイヤーと敵の中間）
    DirectX::XMFLOAT3 focusPoint = {
        (playerPos.x + targetPos.x) * FOCUS_POINT_MIDPOINT,
        playerPos.y + verticalFocusOffset,
        (playerPos.z + targetPos.z) * FOCUS_POINT_MIDPOINT
    };

    // カメラシェイク適用
    if (CameraShake::Instance().IsShaking()) {
        auto shakeOffset = CameraShake::Instance().GetShakeOffset();

        desiredCameraPos.x += shakeOffset.x;
        desiredCameraPos.y += shakeOffset.y;
        desiredCameraPos.z += shakeOffset.z;

        focusPoint.x += shakeOffset.x * SHAKE_FOCUS_FACTOR;
        focusPoint.y += shakeOffset.y * SHAKE_FOCUS_FACTOR;
        focusPoint.z += shakeOffset.z * SHAKE_FOCUS_FACTOR;
    }

    // 補間でスムーズ移動
    DirectX::XMVECTOR cur = XMLoadFloat3(&currentCameraPosition);
    DirectX::XMVECTOR des = XMLoadFloat3(&desiredCameraPos);
    DirectX::XMVECTOR lerped = DirectX::XMVectorLerp(cur, des, smoothness * elapsedTime);
    DirectX::XMStoreFloat3(&currentCameraPosition, lerped);

    // カメラ更新
    Camera::Instance().SetLookAt(
        currentCameraPosition,
        focusPoint,
        { 0, 1, 0 }
    );
}

void LockOnController::ToggleLockOn()
{
    // ロックオンしていない場合
    if (!isLockedOn) {
        // 敵にロックオンする
        Enemy& enemy = Enemy::Instance();
        SetTarget(&enemy);
        SetLockOnEnabled(true);
    }
    else {
        // ロックオン解除
        SetLockOnEnabled(false);
    }
}

void LockOnController::SetLockOnEnabled(bool enabled)
{
    // ロックオン状態を設定
    isLockedOn = enabled;

    // 無効にした場合はターゲットをクリア
    if (!enabled) {
        currentTarget = nullptr;
    }
}

void LockOnController::SetTarget(Character* target)
{
    // ターゲットを設定
    currentTarget = target;
}