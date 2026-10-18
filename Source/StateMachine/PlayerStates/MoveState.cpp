#include <cmath>

#include "MoveState.h"
#include "StateMachine/PlayerStates/AttackStates/PlayerAirAttackState.h"
#include "StateMachine/PlayerStates/AttackStates/PlayerAttackState.h"
#include "StateMachine/PlayerStates/AttackStates/PlayerProjectileAttackState.h"
#include "StateMachine/PlayerStates/AttackStates/PlayerCounterState.h"

// ============================================================================
// PlayerIdleState
// ============================================================================
void PlayerIdleState::Enter(Player& player)
{
    // アイドル状態のアニメーションを開始
    player.StartAnimation(player.GetAnimIndex(PlayerAnim::IDLE_COMBAT), false);
}

void PlayerIdleState::Update(Player& player, float elapsedTime)
{
    // カウンター状態に入れるかチェック
    if (player.CanEnterCounterState() && player.InputDash()) {
        player.GetStateManager().PushState(&PlayerCounterState::Instance());
        return;
    }
    // 近接攻撃
    if (player.InputMeleeAttack()) {
        player.GetStateManager().PushState(&PlayerAttackState::Instance());
        return;
    }
    // 遠距離攻撃 マナチェック
    if (player.InputProjectileAttack() &&
        player.GetCurrentMana() > PlayerConstants::PROJECTILE_ATTACK_MANA_THRESHOLD) {
        player.GetStateManager().PushState(&PlayerProjectileAttackState::Instance());
        return;
    }
    // ジャンプ
    if (player.InputJump() && player.isGround) {
        player.GetStateManager().PushState(&PlayerJumpState::Instance());
        return;
    }
    // ダッシュ
    if (player.InputDash()) {
        player.GetStateManager().PushState(&PlayerDashState::Instance());
        return;
    }
    // 移動入力の取得
    DirectX::XMVECTOR worldDir;
    bool isMoving = player.InputMove(worldDir);
    // 移動中ならラン状態に遷移
    if (isMoving) {
        player.GetStateManager().PushState(&PlayerRunState::Instance());
    }
}

void PlayerIdleState::Exit(Player& player)
{
    // クリーンアップは不要
}

// ============================================================================
// PlayerRunState
// ============================================================================
void PlayerRunState::Enter(Player& player)
{
    // 移動入力の取得
    DirectX::XMVECTOR worldDir;
    bool isMoving = player.InputMove(worldDir);
    // ランの開始アニメーションで開始
    int animIndex = player.GetAnimIndex(PlayerAnim::RUN_START);
    player.StartAnimation(animIndex, false);
    currentPhase = RunPhase::Start;
    phaseTimer = 0.0f;
    phaseDuration = player.model->GetAnimations().at(animIndex).secondsLength;
}

void PlayerRunState::Update(Player& player, float elapsedTime)
{
    // 近接攻撃
    if (player.InputMeleeAttack()) {
        player.GetStateManager().PushState(&PlayerAttackState::Instance());
        return;
    }
    // 遠距離攻撃 マナチェック
    if (player.InputProjectileAttack() &&
        player.GetCurrentMana() > PlayerConstants::PROJECTILE_ATTACK_MANA_THRESHOLD) {
        player.GetStateManager().PopState();
        player.GetStateManager().PushState(&PlayerProjectileAttackState::Instance());
        return;
    }
    // ジャンプ
    if (player.InputJump() && player.isGround) {
        player.GetStateManager().PushState(&PlayerJumpState::Instance());
        return;
    }
    // ダッシュ
    if (player.InputDash()) {
        player.GetStateManager().PushState(&PlayerDashState::Instance());
        return;
    }
    // 移動入力の取得
    DirectX::XMVECTOR worldDir;
    bool isMoving = player.InputMove(worldDir);
    // フェーズタイマー更新
    phaseTimer += elapsedTime;

    switch (currentPhase)
    {
    case RunPhase::Start:
        motionScale = PlayerConstants::RUN_START_MOTION_SCALE;
        if (isMoving) {
            float vx = DirectX::XMVectorGetX(worldDir);
            float vz = DirectX::XMVectorGetZ(worldDir);
            // 滑らかな遷移のため通常より少し遅い回転速度で向きを変える
            player.Turn(elapsedTime, vx, vz, PlayerConstants::TURN_SPEED_NORMAL * PlayerConstants::RUN_START_TURN_SPEED_RATIO);
        }
        else {
            // プレイヤーが早く入力を離した場合 開始アニメーションを加速して早く終了させる
            float accelFactor = PlayerConstants::RUN_START_ACCEL_FACTOR;
            phaseTimer += elapsedTime * (accelFactor - 1.0f);
            if (phaseTimer >= phaseDuration) {
                // 停止アニメーションに遷移
                int stopAnimIndex = player.GetAnimIndex(PlayerAnim::RUN_STOP);
                player.StartAnimation(stopAnimIndex, false);
                currentPhase = RunPhase::Stop;
                phaseTimer = 0.0f;
                phaseDuration = player.model->GetAnimations().at(stopAnimIndex).secondsLength;
            }
            break;
        }
        if (phaseTimer >= phaseDuration) {
            if (isMoving) {
                // ループアニメーションに遷移
                int loopAnimIndex = player.GetAnimIndex(PlayerAnim::RUN_LOOP);
                player.StartAnimation(loopAnimIndex, true);
                currentPhase = RunPhase::Loop;
                phaseTimer = 0.0f;
            }
            else {
                // 直接停止アニメーションに遷移
                int stopAnimIndex = player.GetAnimIndex(PlayerAnim::RUN_STOP);
                player.StartAnimation(stopAnimIndex, false);
                currentPhase = RunPhase::Stop;
                phaseTimer = 0.0f;
                phaseDuration = player.model->GetAnimations().at(stopAnimIndex).secondsLength;
            }
        }
        break;

    case RunPhase::Loop:
        motionScale = PlayerConstants::RUN_LOOP_MOTION_SCALE;
        if (!isMoving) {
            // 停止アニメーションを開始
            int stopAnimIndex = player.GetAnimIndex(PlayerAnim::RUN_STOP);
            player.StartAnimation(stopAnimIndex, false);
            currentPhase = RunPhase::Stop;
            phaseTimer = 0.0f;
            phaseDuration = player.model->GetAnimations().at(stopAnimIndex).secondsLength;
        }
        else {
            float normalizedTime = player.animationSeconds /
                player.model->GetAnimations().at(player.animationIndex).secondsLength;
            int newAnimIndex = player.GetAnimIndex(PlayerAnim::RUN_LOOP);
            if (player.animationIndex != newAnimIndex) {
                player.StartAnimation(newAnimIndex, true);
                // サイクル位置を維持
                player.animationSeconds = normalizedTime *
                    player.model->GetAnimations().at(newAnimIndex).secondsLength;
                player.oldAnimationSeconds = player.animationSeconds;
            }
            float vx = DirectX::XMVectorGetX(worldDir);
            float vz = DirectX::XMVectorGetZ(worldDir);
            player.Turn(elapsedTime, vx, vz, PlayerConstants::TURN_SPEED_NORMAL);
        }
        break;

    case RunPhase::Stop:
        motionScale = PlayerConstants::RUN_STOP_MOTION_SCALE;
        if (isMoving) {
            // 停止をキャンセルしてループに戻る
            player.StartAnimation(player.GetAnimIndex(PlayerAnim::RUN_LOOP), true);
            currentPhase = RunPhase::Loop;
            phaseTimer = 0.0f;
        }
        else if (phaseTimer >= phaseDuration) {
            // 停止完了 アイドル状態へ
            player.GetStateManager().PushState(&PlayerIdleState::Instance());
        }
        break;
    }
}

void PlayerRunState::Exit(Player& player)
{
    // クリーンアップは不要
}

// ============================================================================
// PlayerJumpState
// ============================================================================
void PlayerJumpState::Enter(Player& player)
{
    // 地面にいる場合はダブルジャンプをリセット
    if (player.isGround) {
        hasDoubleJumped = false;
        canDoubleJump = true;
        player.jumpCount = 1;
        // すぐに上昇ブーストを与える
        player.velocity.y = player.jumpPower;
        player.isGround = false;
        player.hasAirDashed = false;
    }
    // ジャンプ開始アニメーションを開始
    player.StartAnimation(player.GetAnimIndex(PlayerAnim::JUMP_START), false);
    currentPhase = JumpPhase::Start;
    phaseTimer = 0.0f;
    phaseDuration = player.model->GetAnimations().at(player.animationIndex).secondsLength;
}

void PlayerJumpState::Update(Player& player, float elapsedTime)
{
    // 空中近接攻撃 マナチェック
    if (player.InputMeleeAttack() && !player.isGround &&
        player.GetCurrentMana() > PlayerConstants::AIR_ATTACK_MANA_THRESHOLD) {
        player.GetStateManager().PushState(&PlayerAirAttackState::Instance());
        return;
    }
    // 空中ダッシュ
    if (player.InputDash() && !player.hasAirDashed) {
        player.GetStateManager().PushState(&PlayerDashState::Instance());
        return;
    }
    phaseTimer += elapsedTime;

    switch (currentPhase)
    {
    case JumpPhase::Start:
        // 開始フェーズ中は軽減された物理演算を適用
        ApplyAirPhysics(player, elapsedTime);
        if (phaseTimer >= phaseDuration) {
            // ループに遷移
            player.StartAnimation(player.GetAnimIndex(PlayerAnim::JUMP_LOOP), true);
            currentPhase = JumpPhase::Loop;
            phaseTimer = 0.0f;
        }
        break;

    case JumpPhase::Loop:
        // ダブルジャンプチェック
        if (player.InputJump() && canDoubleJump && !hasDoubleJumped) {
            hasDoubleJumped = true;
            canDoubleJump = false;
            player.jumpCount++;
            // ダブルジャンプアニメーション
            int doubleJumpIndex = player.GetAnimIndex(PlayerAnim::DOUBLE_JUMP);
            player.StartAnimation(doubleJumpIndex, false);
            // 垂直速度をリセット
            player.velocity.y = player.doubleJumpPower;
            // 滑らかな遷移のために開始フェーズを再開
            currentPhase = JumpPhase::Start;
            phaseTimer = 0.0f;
            phaseDuration = player.model->GetAnimations().at(doubleJumpIndex).secondsLength;
            return;
        }
        // 重力と空中制御を適用
        ApplyAirPhysics(player, elapsedTime);
        // 着地検出
        if (player.position.y <= 0.0f) {
            player.position.y = 0.0f;
            player.velocity.y = 0.0f;
            player.isGround = true;
            // 着地アニメーション
            int landIndex = player.GetAnimIndex(PlayerAnim::JUMP_STOP);
            player.StartAnimation(landIndex, false);
            currentPhase = JumpPhase::Land;
            phaseTimer = 0.0f;
            phaseDuration = player.model->GetAnimations().at(landIndex).secondsLength;
        }
        break;

    case JumpPhase::Land:
        if (phaseTimer >= phaseDuration) {
            DirectX::XMVECTOR worldDir;
            bool isMoving = player.InputMove(worldDir);
            player.TransitionToMovementState(isMoving);
        }
        break;
    }
}

void PlayerJumpState::ApplyAirPhysics(Player& player, float elapsedTime)
{
    // 重力を適用 落下中はより強く
    float gravityMultiplier = (player.velocity.y < 0) ?
        PlayerConstants::FALL_GRAVITY_MULTIPLIER : PlayerConstants::RISE_GRAVITY_MULTIPLIER;
    player.velocity.y += player.gravity * gravityMultiplier * elapsedTime;
    // 空中制御
    DirectX::XMVECTOR worldDir;
    bool isMoving = player.InputMove(worldDir);
    if (isMoving) {
        float vx = DirectX::XMVectorGetX(worldDir);
        float vz = DirectX::XMVectorGetZ(worldDir);
        player.Turn(elapsedTime, vx, vz, PlayerConstants::AIR_TURN_SPEED);
        DirectX::XMFLOAT3 moveDelta;
        DirectX::XMStoreFloat3(&moveDelta,
            DirectX::XMVectorScale(worldDir, PlayerConstants::AIR_CONTROL * PlayerConstants::AIR_SPEED));
        player.velocity.x = player.velocity.x * PlayerConstants::AIR_DRAG + moveDelta.x;
        player.velocity.z = player.velocity.z * PlayerConstants::AIR_DRAG + moveDelta.z;
    }
    else {
        player.velocity.x *= PlayerConstants::AIR_DRAG_IDLE;
        player.velocity.z *= PlayerConstants::AIR_DRAG_IDLE;
    }
    // 位置を更新
    player.position.x += player.velocity.x * elapsedTime;
    player.position.y += player.velocity.y * elapsedTime;
    player.position.z += player.velocity.z * elapsedTime;
}

void PlayerJumpState::Exit(Player& player)
{
    // クリーンアップは不要
}

// ============================================================================
// PlayerDashState
// ============================================================================
void PlayerDashState::Enter(Player& player)
{
    // 移動方向を取得
    DirectX::XMVECTOR worldDir;
    bool isMoving = player.InputMove(worldDir);
    // 空中でダッシュを開始したか追跡
    wasInAir = !player.isGround;
    dashDuration = 0.0f;
    // 空中の場合は空中ダッシュを使用済みにマーク
    if (wasInAir) {
        player.hasAirDashed = true;
        initialAirY = player.position.y;
        initialVerticalVelocity = player.velocity.y;
    }
    PlayerAnim selectedAnim;
    if (player.isGround) {
        selectedAnim = isMoving ? PlayerAnim::DODGE_F : PlayerAnim::DODGE_B;
    }
    else selectedAnim = PlayerAnim::AIR_DODGE_F;
    player.StartAnimation(player.GetAnimIndex(selectedAnim), false);
    // ダッシュ速度を適用
    if (wasInAir) {
        DirectX::XMFLOAT3 dashVel;
        DirectX::XMStoreFloat3(&dashVel, DirectX::XMVectorScale(worldDir, PlayerConstants::AIR_DASH_SPEED));
        player.velocity.x = dashVel.x;
        player.velocity.z = dashVel.z;
    }
    else {
        player.velocity.y = 0.0f;
        DirectX::XMFLOAT3 dashVel;
        DirectX::XMStoreFloat3(&dashVel, DirectX::XMVectorScale(worldDir, PlayerConstants::DASH_SPEED));
        player.velocity.x = dashVel.x;
        player.velocity.z = dashVel.z;
    }
}

void PlayerDashState::Update(Player& player, float elapsedTime)
{
    dashDuration += elapsedTime;
    // 近接攻撃
    if (player.InputMeleeAttack()) {
        bool shouldDoAirAttack = !player.isGround &&
            player.GetCurrentMana() > PlayerConstants::AIR_ATTACK_MANA_THRESHOLD;
        if (shouldDoAirAttack) {
            player.GetStateManager().PopState();
            player.GetStateManager().PushState(&PlayerAirAttackState::Instance());
        }
        else if (player.isGround) {
            player.GetStateManager().PopState();
            player.GetStateManager().PushState(&PlayerAttackState::Instance());
        }
        return;
    }
    // 遠距離攻撃
    if (player.InputProjectileAttack() &&
        player.GetCurrentMana() > PlayerConstants::PROJECTILE_ATTACK_MANA_THRESHOLD) {
        if (player.isGround) {
            player.GetStateManager().PopState();
            player.GetStateManager().PushState(&PlayerProjectileAttackState::Instance());
        }
        return;
    }
    // ダッシュ中の継続的な移動検出
    DirectX::XMVECTOR moveDir;
    bool isMoving = player.InputMove(moveDir);
    if (isMoving) {
        float vx = DirectX::XMVectorGetX(moveDir);
        float vz = DirectX::XMVectorGetZ(moveDir);
        player.Turn(elapsedTime, vx, vz, PlayerConstants::DASH_TURN_SPEED);
        DirectX::XMFLOAT3 dashVel;
        DirectX::XMStoreFloat3(&dashVel, DirectX::XMVectorScale(moveDir, PlayerConstants::DASH_SPEED));
        player.velocity.x = dashVel.x;
        player.velocity.z = dashVel.z;
    }
    if (wasInAir) {
        player.velocity.y += player.gravity * elapsedTime;
    }
    // アニメーションタイマー更新
    player.animationSeconds += elapsedTime * PlayerConstants::DASH_ANIM_SPEED_SCALE;
    // ダッシュアニメーションが完了したかチェック
    const Model::Animation& currentAnim = player.model->GetAnimations().at(player.animationIndex);
    if (player.animationSeconds >= currentAnim.secondsLength) {
        if (wasInAir) {
            player.GetStateManager().PushState(&PlayerFallingState::Instance());
        }
        else {
            player.GetStateManager().PopState();
            DirectX::XMVECTOR worldDir;
            bool isMoving = player.InputMove(worldDir);
            player.TransitionToMovementState(isMoving);
        }
    }
}

void PlayerDashState::Exit(Player& player)
{
    // クリーンアップは不要
}

// ============================================================================
// PlayerFallingState
// ============================================================================
void PlayerFallingState::Enter(Player& player)
{
    // 落下ループアニメーションで開始 ジャンプループと同じ
    player.StartAnimation(player.GetAnimIndex(PlayerAnim::JUMP_LOOP), true);
    currentPhase = FallPhase::Loop;
    phaseTimer = 0.0f;
    // 残っている上昇速度をリセット
    if (player.velocity.y > 0) {
        player.velocity.y = 0.0f;
    }
}

void PlayerFallingState::Update(Player& player, float elapsedTime)
{
    // 空中ダッシュ
    if (player.InputDash() && !player.hasAirDashed) {
        player.GetStateManager().PushState(&PlayerDashState::Instance());
        return;
    }
    phaseTimer += elapsedTime;

    switch (currentPhase)
    {
    case FallPhase::Loop:
        // 落下物理演算を適用
        ApplyFallingPhysics(player, elapsedTime);
        // 着地検出
        if (player.position.y <= 0.0f) {
            player.position.y = 0.0f;
            player.velocity.y = 0.0f;
            player.isGround = true;
            // 着地アニメーション ジャンプ終了
            int landIndex = player.GetAnimIndex(PlayerAnim::JUMP_STOP);
            player.StartAnimation(landIndex, false);
            currentPhase = FallPhase::Land;
            phaseTimer = 0.0f;
            phaseDuration = player.model->GetAnimations().at(landIndex).secondsLength;
        }
        break;

    case FallPhase::Land:
        if (phaseTimer >= phaseDuration) {
            DirectX::XMVECTOR worldDir;
            bool isMoving = player.InputMove(worldDir);
            player.TransitionToMovementState(isMoving);
        }
        break;
    }
}

void PlayerFallingState::ApplyFallingPhysics(Player& player, float elapsedTime)
{
    // 落下中はより強い重力を適用
    player.velocity.y += player.gravity * PlayerConstants::FALL_GRAVITY_MULTIPLIER * elapsedTime;
    // 空中制御 通常のジャンプより低下
    DirectX::XMVECTOR worldDir;
    bool isMoving = player.InputMove(worldDir);
    if (isMoving) {
        float vx = DirectX::XMVectorGetX(worldDir);
        float vz = DirectX::XMVectorGetZ(worldDir);
        player.Turn(elapsedTime, vx, vz, PlayerConstants::AIR_TURN_SPEED);
        DirectX::XMFLOAT3 moveDelta;
        DirectX::XMStoreFloat3(&moveDelta,
            DirectX::XMVectorScale(worldDir, PlayerConstants::AIR_CONTROL * PlayerConstants::AIR_SPEED *
                PlayerConstants::FALLING_AIR_CONTROL_RATIO * elapsedTime));
        player.velocity.x = player.velocity.x * PlayerConstants::FALLING_DRAG + moveDelta.x * PlayerConstants::FALLING_MOVE_DELTA_RATIO;
        player.velocity.z = player.velocity.z * PlayerConstants::FALLING_DRAG + moveDelta.z * PlayerConstants::FALLING_MOVE_DELTA_RATIO;
    }
    else {
        player.velocity.x *= PlayerConstants::FALLING_DRAG_IDLE;
        player.velocity.z *= PlayerConstants::FALLING_DRAG_IDLE;
    }
    // 位置を更新
    player.position.x += player.velocity.x * elapsedTime;
    player.position.y += player.velocity.y * elapsedTime;
    player.position.z += player.velocity.z * elapsedTime;
}

void PlayerFallingState::Exit(Player& player)
{
    // 落下状態を終了するとき空中ダッシュをリセット 着地
    player.hasAirDashed = false;
    player.jumpCount = 0;
}