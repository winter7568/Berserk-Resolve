#include "PlayerAirAttackState.h"
#include "StateMachine/PlayerStates/MoveState.h"
#include "GameLib/Input.h"
#include "GameSystems/HitManagement/HitManager.h"
#include "GameSystems/Time/TimeManager.h"
#include "GameSystems/Particle/ParticleManager.h"
#include "GameSystems/Projectile/ProjectileManager.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameObjects/Player/Player.h"

// ============================================================================
// PlayerAirAttackState - 空中攻撃
// ============================================================================
void PlayerAirAttackState::Enter(Player& player)
{
    // 開始前に最初の攻撃に十分なマナがあるかチェック
    const auto& projData = GetAirProjectileData(CharacterConstants::FIRST_ATTACK);
    auto& projConfig = ProjectileDataManager::Instance().GetConfig(projData.type);
    float totalCost = projConfig.manaCost * projData.projectileCount;
    // 最初の攻撃の全投射物の合計コストを計算
    if (Player::Instance().GetCurrentMana() < totalCost) {
        player.GetStateManager().PopState();
        return;
    }
    currentAttack = CharacterConstants::FIRST_ATTACK;
    queuedInputs = 0;
    inputBufferTime = 0.0f;
    isActive = true;
    hasSpawnedProjectileThisAttack = false;
    projectilesSpawnedThisAttack = 0;
    lastAttackIndex = -1;
    // 武器回転変数をリセット
    isSpinning = false;
    staffForwardOffset = 0.0f;
    player.SetCurrentAttackIndex(currentAttack);
    player.SetCurrentAttackStateID(PlayerStateID::AIR_ATTACK);
    // 最初の攻撃を再生
    player.StartAnimation(player.GetAnimIndex(attackAnimations[currentAttack]), false);
}

void PlayerAirAttackState::Update(Player& player, float elapsedTime)
{
    const AttackData& atk = attackData[currentAttack];
    int currentFrame = static_cast<int>(player.animationSeconds * CharacterConstants::FRAME_RATE);

    // 攻撃4のスローモーションを追加
    UpdateTimeScale(currentAttack);

    // 攻撃変更時にリセット
    ResetSpawnStateIfNeeded(currentAttack);

    // 投射物を生成
    SpawnAirProjectile(player, currentFrame);

    // 武器回転管理 3回目の攻撃専用
    UpdateWeaponSpin(player, currentFrame);

    // キャンセルウィンドウチェック
    if (CheckCancelWindow(player, currentFrame)) {
        return;
    }

    // 空中コンボの再開を防止
    ResetQueuedInputsIfNeeded(currentFrame);

    // アニメーション終了
    if (currentFrame >= atk.totalFrames) {
        FinishAirAttack(player);
    }
}

void PlayerAirAttackState::Exit(Player& player)
{
    // 空中攻撃サウンドを停止
    player.StopPlayerSound(PlayerSound::AIR_SLASH);
    player.StopPlayerSound(PlayerSound::AIR_BEAM);
    if (thirdAttackSoundPlaying) {
        player.StopPlayerSound(PlayerSound::STAFF_SPIN);
        thirdAttackSoundPlaying = false;
    }
    queuedInputs = 0;
    currentAttack = CharacterConstants::FIRST_ATTACK;
    isActive = false;
    player.StopWeaponSpin();
    player.SetCurrentAttackIndex(CharacterConstants::FIRST_ATTACK);
    player.SetCurrentAttackStateID(PlayerStateID::NONE);
    TimeManager::Instance().ResetTimeScale(PlayerConstants::TIME_RESET_DURATION);
}

// ============================================================================
// PlayerAirAttackState - ヘルパー関数
// ============================================================================

void PlayerAirAttackState::UpdateTimeScale(int currentAttack)
{
    // 攻撃4のスローモーションを追加
    if (currentAttack == CharacterConstants::FOURTH_ATTACK) {
        TimeManager::Instance().SetTimeScale(PlayerConstants::TIME_SLOW_FACTOR, PlayerConstants::TIME_SLOW_DURATION);
    }
    else {
        TimeManager::Instance().ResetTimeScale(PlayerConstants::TIME_RESET_DURATION);
    }
}

void PlayerAirAttackState::ResetSpawnStateIfNeeded(int currentAttack)
{
    if (currentAttack != lastAttackIndex) {
        hasSpawnedProjectileThisAttack = false;
        projectilesSpawnedThisAttack = 0;
        lastAttackIndex = currentAttack;
    }
}

void PlayerAirAttackState::SpawnAirProjectile(Player& player, int currentFrame)
{
    const auto& projData = GetAirProjectileData(currentAttack);

    // 投射物を生成する時間かチェック
    if (projectilesSpawnedThisAttack >= projData.projectileCount) {
        return;
    }

    int scheduledFrame = projData.spawnFrame;
    // 遅延をサポート
    if (!projData.delays.empty()) {
        scheduledFrame = static_cast<int>(projData.delays[projectilesSpawnedThisAttack]);
    }
    else if (projectilesSpawnedThisAttack < projData.delays.size()) {
        int frameOffset = static_cast<int>(projData.delays[projectilesSpawnedThisAttack] * CharacterConstants::FRAME_RATE);
        scheduledFrame = projData.spawnFrame + frameOffset;
    }

    // 予定フレームに到達し未生成の場合のみ投射物を生成
    if (currentFrame < scheduledFrame || hasSpawnedProjectileThisAttack) {
        return;
    }

    DirectX::XMFLOAT3 spawnPosition = player.GetWeaponTipPosition();
    DirectX::XMFLOAT3 playerForward = { sinf(player.GetAngle().y), 0.0f, cosf(player.GetAngle().y) };
    DirectX::XMFLOAT3 playerRight = { playerForward.z, 0.0f, -playerForward.x };

    DirectX::XMFLOAT3 slashOrientation = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 spawnOffset = {
        playerForward.x * PlayerConstants::PROJECTILE_SPAWN_FORWARD_OFFSET,
        0.0f,
        playerForward.z * PlayerConstants::PROJECTILE_SPAWN_FORWARD_OFFSET
    };
    DirectX::XMFLOAT3 offsetSpawnPosition = {
        spawnPosition.x + spawnOffset.x,
        spawnPosition.y + spawnOffset.y,
        spawnPosition.z + spawnOffset.z
    };

    if (currentAttack == CharacterConstants::FIRST_ATTACK) {
        player.PlayPlayerSound(airAttackSoundMap[CharacterConstants::FIRST_ATTACK], PlayerConstants::AIR_SLASH_VOLUME);
        // 最初の斬撃 右から左へ水平 負の右方向
        slashOrientation = { -playerRight.x, 0.0f, -playerRight.z };

        auto& projConfig = ProjectileDataManager::Instance().GetConfig(projData.type);
        if (Player::Instance().TryConsumeMana(projConfig.manaCost)) {
            ProjectileManager::Instance().CreateCapsuleProjectile("player", projData.type, offsetSpawnPosition, playerForward, slashOrientation);
        }
        projectilesSpawnedThisAttack++;
        hasSpawnedProjectileThisAttack = true;
    }
    else if (currentAttack == CharacterConstants::SECOND_ATTACK) {
        player.StopPlayerSound(airAttackSoundMap[CharacterConstants::FIRST_ATTACK]);
        player.PlayPlayerSound(airAttackSoundMap[CharacterConstants::SECOND_ATTACK], PlayerConstants::AIR_SLASH_VOLUME);
        // 2回目の斬撃 左から右へ水平 正の右方向
        slashOrientation = { playerRight.x, 0.0f, playerRight.z };

        auto& projConfig = ProjectileDataManager::Instance().GetConfig(projData.type);
        if (Player::Instance().TryConsumeMana(projConfig.manaCost)) {
            ProjectileManager::Instance().CreateCapsuleProjectile("player", projData.type, offsetSpawnPosition, playerForward, slashOrientation);
        }
        projectilesSpawnedThisAttack++;
        hasSpawnedProjectileThisAttack = true;
    }
    else if (currentAttack == CharacterConstants::FOURTH_ATTACK) {
        auto& projConfig = ProjectileDataManager::Instance().GetConfig(projData.type);
        if (!Player::Instance().TryConsumeMana(projConfig.manaCost)) {
            // マナ不足
            projectilesSpawnedThisAttack++;
            hasSpawnedProjectileThisAttack = true;
            return; // ビーム生成をスキップ
        }

        player.PlayPlayerSound(airAttackSoundMap[CharacterConstants::FOURTH_ATTACK], PlayerConstants::AIR_BEAM_VOLUME);

        DirectX::XMFLOAT3 swordTip = player.GetWeaponTipPosition();
        DirectX::XMFLOAT3 enemyPos = Enemy::Instance().GetPosition();
        DirectX::XMFLOAT3 playerPos = player.GetPosition();

        DirectX::XMFLOAT3 beamTarget;
        float beamLength;
        float beamIntensity = 1.0f;

        // 敵までの距離を計算
        float dx = enemyPos.x - playerPos.x;
        float dy = enemyPos.y - playerPos.y;
        float dz = enemyPos.z - playerPos.z;
        float distance = sqrt(dx * dx + dy * dy + dz * dz);

        // 空中戦用の広い視界チェック
        DirectX::XMFLOAT3 playerToEnemy = { dx, dy, dz };
        float playerToEnemyLen = sqrt(playerToEnemy.x * playerToEnemy.x + playerToEnemy.y * playerToEnemy.y + playerToEnemy.z * playerToEnemy.z);
        if (playerToEnemyLen > 0.001f) {
            playerToEnemy.x /= playerToEnemyLen;
            playerToEnemy.y /= playerToEnemyLen;
            playerToEnemy.z /= playerToEnemyLen;
        }

        float visionDot = playerForward.x * playerToEnemy.x + playerForward.y * playerToEnemy.y + playerForward.z * playerToEnemy.z;
        // 空中攻撃用の広い視野角 45度
        const float airFovCosine = cosf(DirectX::XMConvertToRadians(PlayerConstants::BEAM_FOV_DEGREES));

        // 敵が広い視野角内かつ有効範囲内の場合のみターゲット
        if (visionDot >= airFovCosine && distance >= PlayerConstants::BEAM_SWEET_SPOT_MIN_DISTANCE) {
            beamTarget = enemyPos;
            beamTarget.y += PlayerConstants::BEAM_TARGET_Y_OFFSET;
            beamLength = distance;
            beamIntensity = PlayerConstants::BEAM_HIT_INTENSITY;
        }
        else {
            // 敵が視野外または近すぎる ビームは真っ直ぐ前方へ
            beamTarget = {
                swordTip.x + playerForward.x * PlayerConstants::BEAM_MAX_LENGTH,
                swordTip.y + playerForward.y * PlayerConstants::BEAM_MAX_LENGTH,
                swordTip.z + playerForward.z * PlayerConstants::BEAM_MAX_LENGTH
            };
            beamLength = PlayerConstants::BEAM_MAX_LENGTH;
            beamIntensity = PlayerConstants::BEAM_MISS_INTENSITY;
        }

        // 主要パーティクル生成 電撃ビームのビジュアルを作成
        ParticleManager::Instance().SpawnLightningBeam(swordTip, beamTarget, beamIntensity, PlayerConstants::BEAM_DURATION);

        // 追加の衝突検出用にビーム投射物を作成
        auto beam = std::make_shared<Projectile>("player", projData.type, swordTip, DirectX::XMFLOAT3{ 0,0,0 });
        beam->SetBeamConnected(true);
        beam->SetBeamSpawnPoint(swordTip);
        beam->SetBeamTarget(beamTarget);
        beam->SetBeamLength(beamLength);
        ProjectileManager::Instance().AddProjectile(beam);

        projectilesSpawnedThisAttack++;
        hasSpawnedProjectileThisAttack = true;
    }
}

void PlayerAirAttackState::UpdateWeaponSpin(Player& player, int currentFrame)
{
    // 武器回転管理 3回目の攻撃専用
    if (currentAttack != CharacterConstants::THIRD_ATTACK) {
        return;
    }

    int spinStart = attackData[CharacterConstants::THIRD_ATTACK].hitStart;
    int spinEnd = attackData[CharacterConstants::THIRD_ATTACK].hitEnd;
    bool shouldSpin = (currentFrame >= spinStart && currentFrame <= spinEnd);

    if (shouldSpin && !isSpinning) {
        // 回転開始
        isSpinning = true;
        player.StartWeaponSpin(totalSpinRotation);
        // 回転開始時に3回目攻撃のサウンドを再生
        if (!thirdAttackSoundPlaying) {
            player.PlayPlayerSound(airAttackSoundMap[CharacterConstants::THIRD_ATTACK], PlayerConstants::STAFF_SPIN_VOLUME);
            thirdAttackSoundPlaying = true;
        }
    }
    else if (!shouldSpin && isSpinning) {
        // 回転停止
        isSpinning = false;
        player.StopWeaponSpin();
        staffForwardOffset = 0.0f;
        // 回転終了時にサウンドを停止
        if (thirdAttackSoundPlaying) {
            player.StopPlayerSound(airAttackSoundMap[CharacterConstants::THIRD_ATTACK]);
            thirdAttackSoundPlaying = false;
        }
    }

    // 回転中の杖前方移動を制御
    if (isSpinning) {
        // 回転開始の最初の3フレームで前方移動
        if (currentFrame >= spinStart && currentFrame <= spinStart + SPIN_BLEND_FRAMES) {
            float progress = (float)(currentFrame - spinStart) / SPIN_BLEND_DURATION;
            staffForwardOffset = progress * PlayerConstants::STAFF_FORWARD_OFFSET_MAX;
        }
        // 回転終了の最後の3フレームで後方移動
        else if (currentFrame >= spinEnd - SPIN_BLEND_FRAMES && currentFrame <= spinEnd) {
            float progress = (float)(currentFrame - (spinEnd - SPIN_BLEND_FRAMES)) / SPIN_BLEND_DURATION;
            staffForwardOffset = (1.0f - progress) * PlayerConstants::STAFF_FORWARD_OFFSET_MAX;
        }
        // 中間フレームでは最大オフセットを維持
        else if (currentFrame > spinStart + SPIN_BLEND_FRAMES && currentFrame < spinEnd - SPIN_BLEND_FRAMES) {
            staffForwardOffset = PlayerConstants::STAFF_FORWARD_OFFSET_MAX;
        }
        player.SetStaffForwardOffset(staffForwardOffset);
    }
}

bool PlayerAirAttackState::CheckCancelWindow(Player& player, int currentFrame)
{
    const AttackData& atk = attackData[currentAttack];

    bool mouseAirAttack = Input::Instance().GetMouse().GetButtonDown() == Mouse::BTN_LEFT;
    bool controllerAirAttack = (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X) != 0;

    if (!atk.hasCancel || !(mouseAirAttack || controllerAirAttack) ||
        currentFrame < atk.cancelStart || currentFrame > atk.cancelEnd) {
        return false;
    }

    // 次の空中攻撃に十分なマナがあるかチェック
    int nextAttack = currentAttack + 1;
    if (nextAttack < airProjectileData.size()) {
        const auto& nextProjData = GetAirProjectileData(nextAttack);
        auto& nextProjConfig = ProjectileDataManager::Instance().GetConfig(nextProjData.type);
        // 総マナコストを計算
        float totalCost = nextProjConfig.manaCost * nextProjData.projectileCount;

        if (Player::Instance().GetCurrentMana() < totalCost) {
            // 進まず 現在の攻撃を終了させる
            return false;
        }
    }

    // 攻撃変更前に回転を停止
    if (isSpinning) {
        isSpinning = false;
        player.StopWeaponSpin();
        // 3回目攻撃のサウンドが再生中なら停止
        if (thirdAttackSoundPlaying) {
            player.StopPlayerSound(airAttackSoundMap[CharacterConstants::THIRD_ATTACK]);
            thirdAttackSoundPlaying = false;
        }
    }

    queuedInputs--;
    currentAttack++;
    player.SetCurrentAttackIndex(currentAttack);
    player.StartAnimation(player.GetAnimIndex(attackAnimations[currentAttack]), false);
    return true;
}

void PlayerAirAttackState::ResetQueuedInputsIfNeeded(int currentFrame)
{
    const AttackData& atk = attackData[currentAttack];

    // 空中コンボの再開を防止
    if (queuedInputs > 0 && currentFrame > atk.cancelEnd && currentAttack < CharacterConstants::THIRD_ATTACK) {
        // コンボ終了後の空中では何もしない 入力は無視
        queuedInputs = 0;
    }
}

void PlayerAirAttackState::FinishAirAttack(Player& player)
{
    // アニメーション終了時に回転を停止
    if (isSpinning) {
        isSpinning = false;
        player.StopWeaponSpin();
    }

    // コンボ終了
    player.GetStateManager().PopState();

    if (!player.isGround) {
        player.GetStateManager().PushState(&PlayerFallingState::Instance());
    }
    else {
        // 地上では通常通りラン/アイドルに遷移
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
}