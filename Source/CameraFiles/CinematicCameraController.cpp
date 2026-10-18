#include <algorithm>

#include "CameraFiles/CinematicCameraController.h"
#include "CameraFiles/CameraShake.h"

// シネマカメラ開始
void CinematicCameraController::StartSequence(
    const DirectX::XMFLOAT3& playerPos,
    const DirectX::XMFLOAT3& enemyPos)
{
    isActive = true;      // シーケンス開始
    isComplete = false;   // 未完了
    shakeTriggered = false; // シェイク未発動
    timer = 0.0f;         // タイマーリセット

    // プレイヤー位置（頭の高さに補正）
    playerPosition = playerPos;
    playerPosition.y += PLAYER_HEAD_HEIGHT;

    // 敵位置（頭の高さに補正）
    enemyPosition = enemyPos;
    enemyPosition.y += ENEMY_HEAD_HEIGHT;

    // 初期カメラ位置（プレイヤーの後ろ＋上）
    currentEye = playerPosition;
    currentEye.z -= START_BACK_OFFSET; // 後ろ
    currentEye.y += START_HEIGHT_OFFSET; // 上

    // 最初はプレイヤーを見る
    currentFocus = playerPosition;
}

// 毎フレーム更新
void CinematicCameraController::Update(float elapsedTime)
{
    CameraShake::Instance().Update(elapsedTime);

    // 無効または終了なら何もしない
    if (!isActive || isComplete) return;

    timer += elapsedTime;

    // ===== 全体の流れ（約8秒） =====
    // 0-3秒：敵へ移動
    // 3-6秒：敵を見せる（回転）
    // 6-8秒：プレイヤーへ戻る

    // 到達時にシェイク発生
    if (!shakeTriggered && timer >= SHAKE_TRIGGER_TIME) {
        CameraShake::Instance().AddTrauma(SHAKE_TRAUMA);
        shakeTriggered = true;
    }

    // ======================
    // Phase 1：敵へ移動
    // ======================
    if (timer < PHASE1_DURATION) {
        float t = timer / PHASE1_DURATION;

        // スムーズ補間（ease）
        t = t * t * (3.0f - 2.0f * t);

        // カメラ位置（直線ではなく少しカーブ）
        currentEye.x = playerPosition.x + (enemyPosition.x - playerPosition.x) * t;
        currentEye.y = playerPosition.y + PHASE1_Y_OFFSET +
            (enemyPosition.y + ORBIT_HEIGHT_VARIATION - (playerPosition.y + PHASE1_Y_OFFSET)) * t;
        currentEye.z = playerPosition.z - PHASE1_Z_OFFSET +
            (enemyPosition.z + PHASE1_ENEMY_Z_OFFSET - (playerPosition.z - PHASE1_Z_OFFSET)) * t;

        // 敵を見る
        currentFocus = enemyPosition;
        currentFocus.y += FOCUS_HEIGHT_OFFSET;
    }
    // ======================
    // Phase 2：敵をぐるっと見る
    // ======================
    else if (timer < PHASE1_DURATION + PHASE2_DURATION) {

        float holdT = (timer - PHASE1_DURATION) / PHASE2_DURATION;

        // 敵の向き（Yaw）
        constexpr float enemyYaw = DirectX::XMConvertToRadians(ENEMY_YAW_DEGREES);

        // 回転行列
        DirectX::XMMATRIX enemyRotation = DirectX::XMMatrixRotationY(-enemyYaw);

        // 前方向と右方向ベクトル
        DirectX::XMVECTOR enemyForward = DirectX::XMVector3TransformNormal(
            DirectX::XMVectorSet(0, 0, 1, 0), enemyRotation);
        DirectX::XMVECTOR enemyRight = DirectX::XMVector3TransformNormal(
            DirectX::XMVectorSet(1, 0, 0, 0), enemyRotation);

        // 円運動（オービット）
        float angle = holdT * DirectX::XM_2PI * ORBIT_SPEED_FACTOR;
        float orbitX = cosf(angle);
        float orbitZ = sinf(angle);

        // ワールド座標に変換
        DirectX::XMVECTOR rightOffset = DirectX::XMVectorScale(enemyRight, orbitX * ORBIT_RADIUS);
        DirectX::XMVECTOR forwardOffset = DirectX::XMVectorScale(enemyForward, orbitZ * ORBIT_RADIUS);
        DirectX::XMVECTOR orbitOffset = DirectX::XMVectorAdd(rightOffset, forwardOffset);

        // 高さ変化（少し上下する）
        float cameraHeight = enemyPosition.y + ORBIT_HEIGHT_BASE -
            (sinf(holdT * DirectX::XM_PI) * ORBIT_HEIGHT_VARIATION);

        DirectX::XMFLOAT3 orbitPos;
        DirectX::XMStoreFloat3(&orbitPos, orbitOffset);

        currentEye.x = enemyPosition.x + orbitPos.x;
        currentEye.y = cameraHeight;
        currentEye.z = enemyPosition.z + orbitPos.z;

        // 敵を見る（少し下を見る）
        currentFocus = enemyPosition;
        currentFocus.y += ENEMY_FOCUS_HEIGHT;

        // 少し前に寄る
        float forwardT = holdT * FORWARD_MOVE_FACTOR;
        currentEye.x += (enemyPosition.x - currentEye.x) * forwardT;
        currentEye.z += (enemyPosition.z - currentEye.z) * forwardT;
    }
    // ======================
    // Phase 3：プレイヤーへ戻る
    // ======================
    else if (timer < PHASE1_DURATION + PHASE2_DURATION + PHASE3_DURATION) {

        float t = (timer - (PHASE1_DURATION + PHASE2_DURATION)) / PHASE3_DURATION;

        // イーズアウト
        t = 1.0f - (1.0f - t) * (1.0f - t);

        // プレイヤーへ戻る
        currentEye.x = enemyPosition.x + (playerPosition.x - enemyPosition.x) * t;
        currentEye.y = enemyPosition.y + PHASE3_ENEMY_Y_OFFSET +
            ((playerPosition.y + PHASE3_PLAYER_Y_OFFSET) -
                (enemyPosition.y + PHASE3_ENEMY_Y_OFFSET)) * t;

        currentEye.z = enemyPosition.z + PHASE3_ENEMY_Z_OFFSET +
            ((playerPosition.z - PHASE1_Z_OFFSET) -
                (enemyPosition.z + PHASE3_ENEMY_Z_OFFSET)) * t;

        // プレイヤーを見る
        currentFocus = playerPosition;
        currentFocus.y += FOCUS_HEIGHT_OFFSET;
    }
    else {
        // 終了
        isComplete = true;
    }
}

// カメラへ適用
void CinematicCameraController::ApplyToCamera(Camera& camera)
{
    DirectX::XMFLOAT3 finalEye = currentEye;

    // シェイク適用
    if (CameraShake::Instance().IsShaking()) {
        auto shakeOffset = CameraShake::Instance().GetShakeOffset();
        finalEye.x += shakeOffset.x;
        finalEye.y += shakeOffset.y;
        finalEye.z += shakeOffset.z;
    }

    // カメラに設定
    camera.SetLookAt(finalEye, currentFocus, { 0, 1, 0 });
}