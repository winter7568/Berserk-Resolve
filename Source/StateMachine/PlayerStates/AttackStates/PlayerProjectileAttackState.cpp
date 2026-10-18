#include "PlayerProjectileAttackState.h"
#include "PlayerAttackState.h"
#include "StateMachine/PlayerStates/MoveState.h"
#include "StateMachine/PlayerStates/AttackStates/PlayerCounterState.h"
#include "GameLib/Input.h"
#include "GameSystems/Time/TimeManager.h"
#include "GameSystems/Particle/ParticleManager.h"
#include "GameSystems/Projectile/ProjectileData.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/Enemy/Enemy.h"

// ============================================================================
// PlayerProjectileAttackState - 遠距離攻撃
// ============================================================================

void PlayerProjectileAttackState::Enter(Player& player)
{
    // 開始前に最初の攻撃に十分なマナがあるかチェック
    const auto& projData = GetProjectileData(CharacterConstants::FIRST_ATTACK);
    auto& projConfig = ProjectileDataManager::Instance().GetConfig(projData.type);
    // 最初の攻撃の全投射物の合計コストを計算
    float totalCost = projConfig.manaCost * projData.projectileCount;
    if (Player::Instance().GetCurrentMana() < totalCost) {
        // この状態を即座にポップ 入ったばかり
        player.GetStateManager().PopState();
        return;
    }
    currentAttack = CharacterConstants::FIRST_ATTACK;
    queuedInputs = 0;
    inputBufferTime = 0.0f;
    hasSpawnedProjectileThisAttack = false;
    projectilesSpawnedThisAttack = 0;
    lastAttackIndex = currentAttack;
    player.SetCurrentAttackIndex(currentAttack);
    player.SetCurrentAttackStateID(PlayerStateID::PROJECTILE_ATTACK);
    player.StartAnimation(player.GetAnimIndex(attackAnimations[currentAttack]), false);
}

void PlayerProjectileAttackState::Update(Player& player, float elapsedTime)
{
    // 近接攻撃範囲内の場合のみ敵を自動追尾 カメラモードに関係なく
    player.AutoFaceEnemy(elapsedTime, PlayerConstants::PROJ_MAX_AUTO_FACE_DISTANCE);

    // 入力オーバーライドチェック（近接、ジャンプ、ダッシュ）
    if (CheckInputOverrides(player)) {
        return;
    }

    const AttackData& atk = attackData[currentAttack];
    const auto& projData = GetProjectileData(currentAttack);
    int currentFrame = static_cast<int>(player.animationSeconds * CharacterConstants::FRAME_RATE);

    // 攻撃3の時間減速
    UpdateTimeScale(currentAttack);

    // 攻撃変更時にリセット
    ResetSpawnStateIfNeeded(currentAttack);

    // 投射物を生成
    SpawnProjectile(player, currentFrame);

    // キャンセルウィンドウチェック
    if (CheckCancelWindow(player, currentFrame)) {
        return;
    }

    // アニメーション終了
    if (currentFrame >= atk.totalFrames) {
        FinishAttack(player);
    }
}

void PlayerProjectileAttackState::Exit(Player& player)
{
    // 残っている投射物サウンドを停止
    player.StopPlayerSound(PlayerSound::PISTOL);
    player.StopPlayerSound(PlayerSound::STAFF);
    player.StopPlayerSound(PlayerSound::BOW_QUICK);
    player.StopPlayerSound(PlayerSound::BOW_LAUNCH);
    TimeManager::Instance().ResetTimeScale(PlayerConstants::TIME_RESET_DURATION);
    queuedInputs = 0;
    currentAttack = CharacterConstants::FIRST_ATTACK;
    player.SetCurrentAttackIndex(CharacterConstants::FIRST_ATTACK);
    player.SetCurrentAttackStateID(PlayerStateID::NONE);
}

// ============================================================================
// PlayerProjectileAttackState - ヘルパー関数
// ============================================================================

bool PlayerProjectileAttackState::CheckInputOverrides(Player& player)
{
    // カウンター状態に入れるかチェック
    if (player.CanEnterCounterState() && player.InputDash()) {
        player.GetStateManager().PushState(&PlayerCounterState::Instance());
        return true;
    }
    // 近接攻撃入力チェック
    if (player.InputMeleeAttack()) {
        player.GetStateManager().PushState(&PlayerAttackState::Instance());
        return true;
    }
    // ジャンプ入力チェック
    if (player.InputJump() && player.isGround) {
        player.GetStateManager().PopState();
        player.GetStateManager().PushState(&PlayerJumpState::Instance());
        return true;
    }
    // ダッシュ入力チェック
    if (player.InputDash()) {
        player.GetStateManager().PushState(&PlayerDashState::Instance());
        return true;
    }
    return false;
}

void PlayerProjectileAttackState::UpdateTimeScale(int currentAttack)
{
    // 攻撃3の時間減速
    if (currentAttack == CharacterConstants::THIRD_ATTACK) {
        TimeManager::Instance().SetTimeScale(PlayerConstants::TIME_SLOW_FACTOR, PlayerConstants::TIME_SLOW_DURATION);
    }
    else {
        TimeManager::Instance().ResetTimeScale(PlayerConstants::TIME_RESET_DURATION);
    }
}

void PlayerProjectileAttackState::ResetSpawnStateIfNeeded(int currentAttack)
{
    if (currentAttack != lastAttackIndex) {
        hasSpawnedProjectileThisAttack = false;
        projectilesSpawnedThisAttack = 0;
        lastAttackIndex = currentAttack;
    }
}

void PlayerProjectileAttackState::SpawnProjectile(Player& player, int currentFrame)
{
    const auto& projData = GetProjectileData(currentAttack);

    // 次の投射物を生成する時間かチェック
    if (projectilesSpawnedThisAttack >= projData.projectileCount) {
        return;
    }

    int scheduledFrame = projData.spawnFrame;
    // 変更 攻撃3の絶対フレームタイミングをサポート 40 50 60など
    if (!projData.delays.empty()) {
        // 遅延が正確なフレームを表す場合 直接使用
        scheduledFrame = static_cast<int>(projData.delays[projectilesSpawnedThisAttack]);
    }
    else {
        // そうでなければ相対時間オフセットとして扱う 秒
        if (projectilesSpawnedThisAttack < projData.delays.size()) {
            int frameOffset = static_cast<int>(projData.delays[projectilesSpawnedThisAttack] * CharacterConstants::FRAME_RATE);
            scheduledFrame = projData.spawnFrame + frameOffset;
        }
    }

    // 相対 時間ベース と絶対 フレームベース の両方のスタイルをサポート
    if (currentFrame < scheduledFrame) {
        return;
    }

    // スポーンノード位置を動的に取得
    DirectX::XMFLOAT3 spawnPosition;
    if (projData.spawnNode == "hand_l")
        spawnPosition = player.GetLeftHandPosition();
    else if (projData.spawnNode == "staff_tip")
        spawnPosition = player.GetWeaponTipPosition();
    else if (projData.spawnNode == "staff_base")
        spawnPosition = player.GetWeaponBasePosition();
    else
        spawnPosition = player.GetWeaponTipPosition();

    // プレイヤーの向きに基づいて前方方向を計算
    DirectX::XMFLOAT3 forward = { sinf(player.GetAngle().y), 0.0f, cosf(player.GetAngle().y) };

    // 投射物を発射
    auto& projConfig = ProjectileDataManager::Instance().GetConfig(projData.type);
    if (Player::Instance().TryConsumeMana(projConfig.manaCost)) {
        // サウンドマップ配列を使用してサウンドを再生
        if (currentAttack >= CharacterConstants::FIRST_ATTACK && currentAttack <= CharacterConstants::FOURTH_ATTACK) {
            if (currentAttack == CharacterConstants::THIRD_ATTACK && projectilesSpawnedThisAttack == 0) {
                // Bow quick (Attack 3) plays only once at first projectile
                player.PlayPlayerSound(attackSoundMap[CharacterConstants::THIRD_ATTACK], PlayerConstants::AIR_SLASH_VOLUME);
            }
            else if (currentAttack != CharacterConstants::THIRD_ATTACK) {
                // 他の攻撃は通常通り再生
                player.PlayPlayerSound(attackSoundMap[currentAttack], PlayerConstants::AIR_SLASH_VOLUME);
            }
        }
        ProjectileManager::Instance().CreateProjectile("player", projData.type, spawnPosition, forward);
        if (projData.type == "staff") {
            ParticleManager::Instance().StartStaffProjectileRendering();
            ParticleManager::Instance().SpawnStaffProjectile(spawnPosition, forward, PlayerConstants::STAFF_PROJECTILE_SPEED);
        }
    }

    // カウントを増加
    projectilesSpawnedThisAttack++;
    // 全ての弾を発射したら完了とマーク
    if (projectilesSpawnedThisAttack >= projData.projectileCount)
        hasSpawnedProjectileThisAttack = true;
}

bool PlayerProjectileAttackState::CheckCancelWindow(Player& player, int currentFrame)
{
    const AttackData& atk = attackData[currentAttack];

    bool mouseProjAttack = Input::Instance().GetMouse().GetButtonDown() == Mouse::BTN_RIGHT;
    bool controllerProjAttack = (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_Y) != 0;

    if (!atk.hasCancel || !(mouseProjAttack || controllerProjAttack) ||
        currentFrame < atk.cancelStart || currentFrame > atk.cancelEnd) {
        return false;
    }

    // 次の攻撃に十分なマナがあるかチェック
    int nextAttack = currentAttack + 1;
    if (nextAttack < projectileData.size()) {
        const auto& nextProjData = GetProjectileData(nextAttack);
        auto& nextProjConfig = ProjectileDataManager::Instance().GetConfig(nextProjData.type);
        // 総マナコストを計算
        float totalCost = nextProjConfig.manaCost * nextProjData.projectileCount;
        if (Player::Instance().GetCurrentMana() < totalCost) {
            // 次の攻撃に進まず 現在の攻撃を終了させる
            return false;
        }
    }

    queuedInputs--;
    currentAttack++;
    player.SetCurrentAttackIndex(currentAttack);
    hasSpawnedProjectileThisAttack = false;
    projectilesSpawnedThisAttack = 0;
    lastAttackIndex = currentAttack;
    player.StartAnimation(player.GetAnimIndex(attackAnimations[currentAttack]), false);
    return true;
}

void PlayerProjectileAttackState::FinishAttack(Player& player)
{
    // コンボ終了
    player.GetStateManager().PopState();
    DirectX::XMVECTOR worldDir;
    bool isMoving = player.InputMove(worldDir);
    if (isMoving) {
        player.model->PlayAnimation(player.GetAnimIndex(PlayerAnim::RUN_START), true, player.animationBlendSeconds);
        player.GetStateManager().PushState(&PlayerRunState::Instance());
    }
    else {
        player.model->PlayAnimation(player.GetAnimIndex(PlayerAnim::IDLE_COMBAT), true, player.animationBlendSeconds);
        player.GetStateManager().PushState(&PlayerIdleState::Instance());
    }
}