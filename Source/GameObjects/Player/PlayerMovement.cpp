#include "Player.h"
#include "GameObjects/Enemy/Enemy.h"
#include "StateMachine/PlayerStates/MoveState.h"
#include "GameLib/Input.h"
#include "GameLib/GamePad.h"
#include "CameraFiles/Camera.h"

// ============================================================================
// PUBLIC METHODS - Input
// ============================================================================
bool Player::InputMove(DirectX::XMVECTOR& outWorldDir)
{
    auto& input = Input::Instance();
    auto& gamePad = input.GetGamePad();

    // キーボード入力
    float keyboardX = 0.0f, keyboardZ = 0.0f;
    if (input.IsKeyDown('W')) keyboardZ += 1.0f;
    if (input.IsKeyDown('S')) keyboardZ -= 1.0f;
    if (input.IsKeyDown('D')) keyboardX += 1.0f;
    if (input.IsKeyDown('A')) keyboardX -= 1.0f;

    // コントローラー入力
    float controllerX = gamePad.GetAxisLX();
    float controllerZ = gamePad.GetAxisLY();

    // 十字キー
    GamePadButton buttons = gamePad.GetButton();
    if (buttons & GamePad::BTN_UP)    controllerZ += 1.0f;
    if (buttons & GamePad::BTN_DOWN)  controllerZ -= 1.0f;
    if (buttons & GamePad::BTN_RIGHT) controllerX += 1.0f;
    if (buttons & GamePad::BTN_LEFT)  controllerX -= 1.0f;

    // 合成 
    float rawX = keyboardX + controllerX;
    float rawZ = keyboardZ + controllerZ;

    // デッドゾーン（コントローラーの小さい入力を無視）
    float controllerMagnitude = sqrtf(controllerX * controllerX + controllerZ * controllerZ);
    if (controllerMagnitude < PlayerConstants::INPUT_DEAD_ZONE)
    {
        rawX = keyboardX;
        rawZ = keyboardZ;
    }

    // 入力チェック
    float totalMagnitude = sqrtf(rawX * rawX + rawZ * rawZ);
    if (totalMagnitude < PlayerConstants::INPUT_DEAD_ZONE)
    {
        outWorldDir = DirectX::XMVectorZero();
        return false;
    }

    // 正規化
    rawX /= totalMagnitude;
    rawZ /= totalMagnitude;

    // カメラ基準のワールド方向に変換
    DirectX::XMVECTOR camForward = DirectX::XMLoadFloat3(&Camera::Instance().GetFront());
    DirectX::XMVECTOR camRight = DirectX::XMLoadFloat3(&Camera::Instance().GetRight());

    camForward = DirectX::XMVectorSetY(camForward, 0.0f);
    camRight = DirectX::XMVectorSetY(camRight, 0.0f);
    camForward = DirectX::XMVector3Normalize(camForward);
    camRight = DirectX::XMVector3Normalize(camRight);

    outWorldDir = DirectX::XMVectorAdd(
        DirectX::XMVectorScale(camForward, rawZ),
        DirectX::XMVectorScale(camRight, rawX)
    );
    outWorldDir = DirectX::XMVector3Normalize(outWorldDir);
    return true;
}

bool Player::InputMeleeAttack()
{
    auto& input = Input::Instance();
    return input.IsGamePadButtonPressed(GamePad::BTN_X) ||
        (input.GetMouse().GetButtonDown() & Mouse::BTN_LEFT);
}

bool Player::InputProjectileAttack()
{
    auto& input = Input::Instance();
    return input.IsGamePadButtonPressed(GamePad::BTN_Y) ||
        (input.GetMouse().GetButtonDown() & Mouse::BTN_RIGHT);
}

bool Player::InputAttack()
{
    return InputMeleeAttack() || InputProjectileAttack();
}

bool Player::InputJump()
{
    auto& input = Input::Instance();
    return input.IsKeyPressed(VK_SPACE) ||
        input.IsGamePadButtonPressed(GamePad::BTN_A);
}

bool Player::InputDash()
{
    auto& input = Input::Instance();
    return input.IsKeyPressed(VK_SHIFT) ||
        input.IsGamePadButtonPressed(GamePad::BTN_B);
}

bool Player::InputLockOn()
{
    auto& input = Input::Instance();

    // Qキーか右ショルダーボタンが押された瞬間
    bool keyPressed = input.IsKeyPressed('Q');
    bool buttonPressed = input.IsGamePadButtonPressed(GamePad::BTN_RIGHT_SHOULDER);

    if (keyPressed || buttonPressed)
    {
        if (!isLockOnActive)
        {
            // ロックオンを試みる
            Enemy& enemy = Enemy::Instance();
            DirectX::XMFLOAT3 playerPos = GetPosition();
            DirectX::XMFLOAT3 enemyPos = enemy.GetPosition();

            float dx = playerPos.x - enemyPos.x;
            float dz = playerPos.z - enemyPos.z;
            float distSq = dx * dx + dz * dz;

            float maxDist = PlayerConstants::MAX_LOCKON_DISTANCE;
            if (distSq <= (maxDist * maxDist))
            {
                isLockOnActive = true;
            }
        }
        else
        {
            isLockOnActive = false;
        }
    }

    return isLockOnActive;
}

// ============================================================================
// PUBLIC METHODS - Movement
// ============================================================================

float Player::GetDashScale(float elapsedTime)
{
    // ダッシュスケール計算用定数
    constexpr float SCALE_RANGE = PlayerConstants::DASH_MAX_SCALE - PlayerConstants::DASH_MIN_SCALE;
    bool isShiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    bool isBPressed = (Input::Instance().GetGamePad().GetButton() & GamePad::BTN_B) != 0;
    bool isButtonPressed = isShiftPressed || isBPressed;
    // 現在ダッシュ状態かチェック
    bool isDashing = (stateManager.IsInState(PlayerStateID::DASH));

    if (isDashing) {
        // ダッシュ中にボタンを押し続けると倍率が上がる
        if (isButtonPressed) {
            dashHoldTime += elapsedTime;
            if (dashHoldTime > PlayerConstants::DASH_MAX_HOLD_TIME) dashHoldTime = PlayerConstants::DASH_MAX_HOLD_TIME;
        }
        // 倍率を計算：ホールド時間に応じて1倍?4倍（0.5秒で4倍）
        float dashScale = PlayerConstants::DASH_MIN_SCALE + (SCALE_RANGE * (dashHoldTime / PlayerConstants::DASH_MAX_HOLD_TIME));
        return dashScale;
    }
    else {
        // ダッシュしていない時はホールド時間をリセット
        dashHoldTime = 0.0f;
        return PlayerConstants::DASH_MIN_SCALE;
    }
}

void Player::UpdateLockOnByDistance()
{
    if (!isLockOnActive) return;

    Enemy& enemy = Enemy::Instance();
    DirectX::XMFLOAT3 playerPos = GetPosition();
    DirectX::XMFLOAT3 enemyPos = enemy.GetPosition();
    // 距離の二乗を計算
    float dx = playerPos.x - enemyPos.x;
    float dz = playerPos.z - enemyPos.z;
    float dy = playerPos.y - enemyPos.y;
    float distSq = dx * dx + dz * dz + dy * dy;

    // 最大距離より遠くなったらロックオンを解除する
    if (distSq > (PlayerConstants::MAX_LOCKON_DISTANCE * PlayerConstants::MAX_LOCKON_DISTANCE))
    {
        isLockOnActive = false;
    }
}

void Player::AutoFaceEnemy(float elapsedTime, float maxDistance)
{
    Enemy& enemy = Enemy::Instance();
    DirectX::XMFLOAT3 playerPos = GetPosition();
    DirectX::XMFLOAT3 enemyPos = enemy.GetPosition();
    float dx = enemyPos.x - playerPos.x;
    float dz = enemyPos.z - playerPos.z;
    float distance = std::sqrt(dx * dx + dz * dz);

    if (distance <= maxDistance) {
        float targetAngle = std::atan2(dx, dz);
        float angleDiff = targetAngle - angle.y;
        while (angleDiff > DirectX::XM_PI) angleDiff -= DirectX::XM_2PI;
        while (angleDiff < -DirectX::XM_PI) angleDiff += DirectX::XM_2PI;
        if (fabs(angleDiff) > DirectX::XM_PI * PlayerConstants::AIR_DRAG) {
            dx = -dz;
            dz = dx;
        }
        Turn(elapsedTime, dx, dz, PlayerConstants::AUTO_FACE_TURNSPEED);
    }
}

void Player::TransitionToMovementState(bool isMoving)
{
    GetStateManager().PopState();
    if (isMoving)
        GetStateManager().PushState(&PlayerRunState::Instance());
    else
        GetStateManager().PushState(&PlayerIdleState::Instance());
}

// ============================================================================
// PROTECTED METHODS - Overrides
// ============================================================================

void Player::RootMotion(float elapsedTime)
{
    if (animationIndex < 0)
        return;
    // ジャンプ中のルートモーションをスキップ
    if (stateManager.IsInState(PlayerStateID::JUMP)) return;
    // アニメーションのルートモーションを処理
    if (animationIndex >= 0)
    {
        const Model::Animation& animation = model->GetAnimations().at(animationIndex);
        // 現在時間のアニメーションポーズを取得
        model->ComputeAnimation(animationIndex, animationSeconds, nodePoses);
        // ルートモーションの移動量を計算
        CalculateRootMotion(animation, elapsedTime);
        // 新しいノードポーズをモデルに反映
        model->SetNodePoses(nodePoses);
    }
}

// ============================================================================
// PRIVATE METHODS - Update Helpers
// ============================================================================

void Player::CalculateRootMotion(const Model::Animation& animation, float elapsedTime)
{
    // ルートボーンのインデックスを取得
    const int rootMotionNodeIndex = model->GetNodeIndex("root");
    // 開始・前フレーム・現在のポーズを取得
    Model::NodeCache beginPose, oldPose, newPose;
    model->ComputeAnimation(animationIndex, rootMotionNodeIndex, 0.0f, beginPose);
    model->ComputeAnimation(animationIndex, rootMotionNodeIndex, oldAnimationSeconds, oldPose);
    model->ComputeAnimation(animationIndex, rootMotionNodeIndex, animationSeconds, newPose);
    // ローカル移動量を計算
    DirectX::XMFLOAT3 localTranslation{};
    // アニメーションループ時の処理
    if (oldAnimationSeconds > animationSeconds)
    {
        // アニメーション終了時のポーズを取得
        Model::NodeCache endPose;
        model->ComputeAnimation(animationIndex, rootMotionNodeIndex, animation.secondsLength, endPose);

        // ループ境界をまたぐ移動量を計算（開始→終了 + 終了→現在）
        localTranslation.x = (newPose.position.x - beginPose.position.x) + (endPose.position.x - oldPose.position.x);
        localTranslation.y = (newPose.position.y - beginPose.position.y) + (endPose.position.y - oldPose.position.y);
        localTranslation.z = (newPose.position.z - beginPose.position.z) + (endPose.position.z - oldPose.position.z);
    }
    else
    {
        // 通常のフレーム間移動量
        localTranslation = {
            newPose.position.x - oldPose.position.x,
            newPose.position.y - oldPose.position.y,
            newPose.position.z - oldPose.position.z,
        };
    }
    DirectX::XMVECTOR LocalTranslation = DirectX::XMLoadFloat3(&localTranslation);
    // ルートノードを開始ポーズにリセット
    nodePoses[rootMotionNodeIndex].position = beginPose.position;
    // ローカル移動量をワールド空間に変換
    DirectX::XMMATRIX worldTransform = DirectX::XMLoadFloat4x4(&transform);
    DirectX::XMVECTOR worldTranslation = DirectX::XMVector3TransformNormal(LocalTranslation, worldTransform);
    // 移動量を格納
    DirectX::XMFLOAT3 motionDelta;
    DirectX::XMStoreFloat3(&motionDelta, worldTranslation);
    // ダッシュ中の移動量倍率を適用
    ApplyDashMotionScale(motionDelta, elapsedTime);
    // 走行中の移動量倍率を適用
    ApplyRunMotionScale(motionDelta);
    // 位置に移動を反映
    position.x += motionDelta.x;
    position.y += motionDelta.y;
    position.z += motionDelta.z;
}

void Player::ApplyDashMotionScale(DirectX::XMFLOAT3& motionDelta, float elapsedTime)
{
    float dashDistanceScale = GetDashScale(elapsedTime);
    bool isDashing = stateManager.IsInState(PlayerStateID::DASH);
    if (isDashing)
    {
        motionDelta.x *= dashDistanceScale;
        motionDelta.z *= dashDistanceScale;
        if (isGround) motionDelta.y *= dashDistanceScale;
        else motionDelta.y = 0;  // 空中ではY軸移動を無効
    }
}

void Player::ApplyRunMotionScale(DirectX::XMFLOAT3& motionDelta)
{
    bool isRunning = stateManager.IsInState(PlayerStateID::RUN);
    if (isRunning)
    {
        float scale = PlayerRunState::Instance().GetMotionScale();
        motionDelta.x *= scale;
        motionDelta.z *= scale;
    }
}