#include "BerserkState.h"
#include "GameSystems/Particle/ParticleManager.h"
#include "GameSystems/Time/TimeManager.h"
#include "CameraFiles/CameraShake.h"
#include "GameSystems/Effect/EffectManager.h"
#include "GameSystems/Projectile/ProjectileManager.h"
#include "GameObjects/Arena/Arena.h"

void BerserkState::Enter(Enemy& enemy) {
    // バーサク開始の音
    enemy.PlayEnemySound(EnemySound::BERSERK_ON, EnemyConstants::BERSERK_BERSERK_ON_VOLUME);

    currentPhase = BerserkPhase::JUMP_START;
    phaseTimer = 0.0f;

    // バーサクの全体時間
    berserkDuration = EnemyConstants::BERSERK_DURATION;

    hasScaledUp = false;

    // 開始時の高さに移動
    enemy.position.y = EnemyConstants::BERSERK_START_Y_POSITION;

    // フィールドを赤く光らせる
    Arena::Instance().FlashEmissive(EnemyConstants::BERSERK_EMISSIVE_COLOR, berserkDuration);

    // テレポート用の位置を保存
    teleportStartPos = { enemy.position.x, enemy.position.y + EnemyConstants::BERSERK_TELEPORT_Y_OFFSET, enemy.position.z };
    teleportEndPos = { berserkPosition.x, berserkPosition.y + EnemyConstants::BERSERK_TELEPORT_Y_OFFSET, berserkPosition.z };

    // 時間を遅くする
    TimeManager::Instance().SetTimeScale(EnemyConstants::BERSERK_TIME_SCALE, EnemyConstants::BERSERK_TIME_SCALE_DURATION);

    // ジャンプ開始アニメーション
    enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::JUMP_START), false);
}


void BerserkState::Update(Enemy& enemy, float elapsedTime) {
    // プレイヤーが危険エリアにいるか判定
    playerIsInZoneThisFrame = IsPlayerInDangerZone();

    // 危険エリアのエフェクト管理
    ManageDangerZoneEffect(playerIsInZoneThisFrame, elapsedTime);

    playerWasInZoneLastFrame = playerIsInZoneThisFrame;

    // プレイヤー死亡時は音を止める
    if (Player::Instance().IsDead())
        enemy.StopEnemySound(EnemySound::BERSERK_ON);

    phaseTimer += elapsedTime;

    switch (currentPhase) {
    case BerserkPhase::JUMP_START:
        UpdateJumpStart(enemy);
        break;

    case BerserkPhase::INVISIBLE_LOOP:
        UpdateInvisibleLoop(enemy);
        break;

    case BerserkPhase::BERSERK_LAND:
        UpdateBerserkLand(enemy);
        break;

    case BerserkPhase::BERSERK_IDLE:
        UpdateBerserkIdle(enemy, elapsedTime);
        break;

    case BerserkPhase::AIM_START:
        UpdateAimStart(enemy);
        break;

    case BerserkPhase::AIM_LOOP:
        UpdateAimLoop(enemy);
        break;

    case BerserkPhase::ATTACK_EXECUTION:
        UpdateAttackExecution(enemy, elapsedTime);
        break;
    }
}

void BerserkState::Exit(Enemy& enemy) {

    // 時間スケールを元に戻す
    TimeManager::Instance().ResetTimeScale(1.0f);

    // エフェクト停止
    if (blastEffectActive && blastEffectHandle != -1) {
        EffectManager::Instance().StopEffect(blastEffectHandle);
        blastEffectActive = false;
        blastEffectHandle = -1;
    }

    // 音を止める
    enemy.StopEnemySound(EnemySound::BERSERK_ON);
    enemy.StopEnemySound(EnemySound::METEOR_SPAWN);
    enemy.StopEnemySound(EnemySound::METEOR_STRIKE);
    enemy.StopEnemySound(EnemySound::SPAWN);
    enemy.StopEnemySound(EnemySound::AREA_WARNING);
    Player::Instance().StopPlayerSound(PlayerSound::HIT_BLAST);

    // 危険エリアのエフェクト停止
    StopDangerZoneEffect();
    playerWasInZoneLastFrame = false;
    playerIsInZoneThisFrame = false;

    // ステージの見た目を戻す
    Arena::Instance().ResetEmissiveColor();

    // 見た目と姿勢を元に戻す
    enemy.SetVisibility(true);
    enemy.ApplyDefaultSwordPose();
}

// ============================================================================
// BerserkState - フェーズ更新ヘルパー
// ============================================================================

void BerserkState::UpdateJumpStart(Enemy& enemy) {
    const auto& anim = enemy.model->GetAnimations().at(enemy.animationIndex);

    // ジャンプ開始が終わったら次へ
    if (phaseTimer >= anim.secondsLength) {
        currentPhase = BerserkPhase::INVISIBLE_LOOP;
        phaseTimer = 0.0f;

        enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::JUMP_LOOP), true);

        // 中央に移動
        enemy.position = berserkPosition;

        FacePlayer(enemy);

        // テレポート演出
        ParticleManager::Instance().SpawnFullSequenceForward(teleportStartPos);
        ParticleManager::Instance().SpawnFullSequenceReverse(teleportEndPos);

        // 一時的に非表示
        enemy.SetVisibility(false);
    }
}

void BerserkState::UpdateInvisibleLoop(Enemy& enemy) {
    // 一度だけサイズ変更
    if (!hasScaledUp) {
        enemy.scale = { EnemyConstants::BERSERK_SCALE, EnemyConstants::BERSERK_SCALE, EnemyConstants::BERSERK_SCALE };
        enemy.weight = EnemyConstants::BERSERK_WEIGHT;

        enemy.SetbodyGlow(true);

        // 武器の当たり判定を強化
        enemy.GetWeaponCapsuleParams().radius *= EnemyConstants::BERSERK_WEAPON_RADIUS_MULTIPLIER;
        enemy.GetWeaponCapsuleParams().height *= EnemyConstants::BERSERK_WEAPON_HEIGHT_MULTIPLIER;

        enemy.ApplyBerserkSwordPose();

        hasScaledUp = true;
    }

    const auto& loopAnim = enemy.model->GetAnimations().at(enemy.animationIndex);

    // ループ終了で着地へ
    if (phaseTimer >= loopAnim.secondsLength) {
        currentPhase = BerserkPhase::BERSERK_LAND;
        phaseTimer = 0.0f;

        enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::B_JUMP_END), false);

        // 再表示
        enemy.SetVisibility(true);

        // 時間を元に戻す
        TimeManager::Instance().ResetTimeScale(1.0f);
    }
}

void BerserkState::UpdateBerserkLand(Enemy& enemy) {
    const auto& anim = enemy.model->GetAnimations().at(enemy.animationIndex);

    // 着地終了で待機
    if (phaseTimer >= anim.secondsLength) {
        currentPhase = BerserkPhase::BERSERK_IDLE;
        phaseTimer = 0.0f;

        enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::B_IDLE), true);
    }
}

void BerserkState::UpdateBerserkIdle(Enemy& enemy, float elapsedTime) {
    // 残り時間を減らす
    berserkDuration -= elapsedTime;

    // フィールドの発光を揺らす
    static float pulseTimer = 0.0f;
    pulseTimer += elapsedTime;

    float pulse = (sin(pulseTimer * EnemyConstants::BERSERK_PULSE_SPEED) + 1.0f) * 0.5f;

    DirectX::XMFLOAT3 pulsedColor{};
    pulsedColor.x = EnemyConstants::BERSERK_PULSE_R_BASE + pulse * EnemyConstants::BERSERK_PULSE_R_VARY;
    pulsedColor.y = EnemyConstants::BERSERK_PULSE_G_BASE + pulse * EnemyConstants::BERSERK_PULSE_G_VARY;
    pulsedColor.z = EnemyConstants::BERSERK_PULSE_B_BASE + pulse * EnemyConstants::BERSERK_PULSE_B_VARY;
    Arena::Instance().SetEmissiveColor(pulsedColor);

    // 時間切れで終了
    if (berserkDuration <= 0.0f) {
        enemy.GetStateManager().PopState();
        return;
    }

    FacePlayer(enemy);

    // 一定時間後に攻撃準備
    if (phaseTimer >= idleToAttackTime) {
        currentPhase = BerserkPhase::AIM_START;
        phaseTimer = 0.0f;

        enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::AIM_START), false);
    }
}

void BerserkState::UpdateAimStart(Enemy& enemy) {
    FacePlayer(enemy);

    const auto& anim = enemy.model->GetAnimations().at(enemy.animationIndex);

    // 構え終わり
    if (phaseTimer >= anim.secondsLength) {
        currentPhase = BerserkPhase::AIM_LOOP;
        phaseTimer = 0.0f;

        enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::AIM_LOOP), true);
    }
}

void BerserkState::UpdateAimLoop(Enemy& enemy) {
    FacePlayer(enemy);

    // 一定時間後に攻撃開始
    if (phaseTimer >= EnemyConstants::BERSERK_AIM_LOOP_DURATION) {
        currentPhase = BerserkPhase::ATTACK_EXECUTION;
        phaseTimer = 0.0f;

        // 攻撃パターンをランダムに選ぶ
        currentAttackSet = rand() % EnemyConstants::BERSERK_ATTACK_COUNT;
        currentAttackPart = EnemyConstants::ATTACK_PART_ONE;

        StartAttackSequence(enemy);
    }
}

void BerserkState::UpdateAttackExecution(Enemy& enemy, float elapsedTime) {
    // 常にプレイヤーの方向を向く
    FacePlayer(enemy);

    const auto& anim = enemy.model->GetAnimations().at(enemy.animationIndex);

    // 第二攻撃セットの処理
    if (currentAttackSet == CharacterConstants::SECOND_ATTACK) {
        UpdateSecondAttackSet(enemy, anim, elapsedTime);
    }

    // 第一攻撃セット メテオの予兆
    if (currentAttackSet == CharacterConstants::FIRST_ATTACK &&
        currentAttackPart == EnemyConstants::ATTACK_PART_ONE &&
        !hasTriggeredSlashEffect) {
        UpdateFirstAttackSet(enemy, anim);
    }

    // メテオ着弾処理
    if (hasTriggeredSlashEffect && !hasSpawnedCollision && phaseTimer >= meteorImpactTime) {
        SpawnMeteorImpact(enemy);
    }

    // 手元にエフェクトを出す
    if (currentAttackSet == CharacterConstants::FIRST_ATTACK &&
        currentAttackPart == EnemyConstants::ATTACK_PART_ONE &&
        !hasTriggeredBlastEffect) {
        SpawnBlastEffect(enemy, anim);
    }

    // エフェクトの位置を更新
    if (blastEffectActive && blastEffectHandle != -1) {
        UpdateBlastEffectPosition(enemy);
    }

    // 次の攻撃段階へ進む判定
    if (currentAttackPart == EnemyConstants::ATTACK_PART_ONE && !isTransitioning) {
        CheckAttackPartTransition(enemy);
    }

    // アニメーション終了時の処理
    if (phaseTimer >= anim.secondsLength) {
        HandleAttackAnimationEnd(enemy);
    }
}

// ============================================================================
// BerserkState - 攻撃実行ヘルパー
// ============================================================================

void BerserkState::UpdateSecondAttackSet(Enemy& enemy, const Model::Animation& anim, float elapsedTime) {
    // 攻撃の最初に警告エリアを表示
    if (currentAttackPart == EnemyConstants::ATTACK_PART_ONE && !hasSpawnedWarningArea) {
        Player& player = Player::Instance();
        warningAreaPos = player.GetPosition();
        warningAreaPos.y = EnemyConstants::BERSERK_WARNING_AREA_Y;

        EffectManager::Instance().PlayEffect(
            "LightWarningArea",
            warningAreaPos,
            EnemyConstants::BERSERK_WARNING_AREA_SCALE
        );

        hasSpawnedWarningArea = true;
    }
    // 次の段階で雷エフェクトを出す
    else if (currentAttackPart == EnemyConstants::ATTACK_PART_TWO && !hasSpawnedSlashLightning) {
        if (phaseTimer >= anim.secondsLength * EnemyConstants::BERSERK_LIGHTNING_SPAWN_PERCENT) {
            EffectManager::Instance().PlayEffect(
                "enemySpawn",
                warningAreaPos,
                EnemyConstants::BERSERK_SPAWN_EFFECT_SCALE
            );

            Enemy::Instance().PlayEnemySound(EnemySound::SPAWN, EnemyConstants::BERSERK_SPAWN_SOUND_VOLUME);

            // 少し遅らせて弾を出す
            projectileDelayTimer = EnemyConstants::BERSERK_PROJECTILE_DELAY;
            hasSpawnedSlashLightning = true;
        }
    }

    // 遅延後に攻撃判定を出す
    if (hasSpawnedSlashLightning && !hasSpawnedSlashProjectile) {
        projectileDelayTimer -= elapsedTime;

        if (projectileDelayTimer <= 0.0f) {
            ProjectileManager::Instance().CreateProjectile(
                "enemy",
                "berserk_danger_zone_hit",
                warningAreaPos,
                { 0, 0, 0 }
            );

            CameraShake::Instance().AddTrauma(EnemyConstants::BERSERK_CAMERA_SHAKE_TRAUMA);

            hasSpawnedSlashProjectile = true;
        }
    }
}

void BerserkState::UpdateFirstAttackSet(Enemy& enemy, const Model::Animation& anim) {
    if (phaseTimer >= anim.secondsLength * EnemyConstants::BERSERK_METEOR_SPAWN_PERCENT) {
        Player& player = Player::Instance();

        meteorTargetPos = player.GetPosition();
        meteorTargetPos.y = 0.0f;

        enemy.PlayEnemySound(EnemySound::METEOR_SPAWN, EnemyConstants::BERSERK_METEOR_SPAWN_VOLUME);

        EffectManager::Instance().PlayEffect(
            "enemyRangedMeteor",
            meteorTargetPos,
            EnemyConstants::BERSERK_METEOR_EFFECT_SCALE
        );

        // 着弾タイミングを記録
        meteorImpactTime = phaseTimer + EnemyConstants::BERSERK_METEOR_IMPACT_DELAY;
        hasTriggeredSlashEffect = true;
    }
}

void BerserkState::SpawnMeteorImpact(Enemy& enemy) {
    enemy.PlayEnemySound(EnemySound::METEOR_STRIKE, EnemyConstants::BERSERK_METEOR_STRIKE_VOLUME);

    ProjectileManager::Instance().CreateProjectile(
        "enemy",
        "berserk_meteor_impact",
        meteorTargetPos,
        { 0, 0, 0 }
    );

    CameraShake::Instance().AddTrauma(EnemyConstants::BERSERK_CAMERA_SHAKE_TRAUMA);

    hasSpawnedCollision = true;
}

void BerserkState::SpawnBlastEffect(Enemy& enemy, const Model::Animation& anim) {
    if (phaseTimer >= anim.secondsLength * EnemyConstants::BERSERK_BLAST_SPAWN_PERCENT) {
        DirectX::XMFLOAT3 handPos = enemy.GetLeftHandPosition();
        DirectX::XMFLOAT3 enemyForward = enemy.GetForwardVector();

        handPos.x += enemyForward.x * EnemyConstants::BERSERK_HAND_FORWARD_OFFSET;
        handPos.z += enemyForward.z * EnemyConstants::BERSERK_HAND_FORWARD_OFFSET;

        blastEffectHandle = EffectManager::Instance().PlayEffectWithHandle(
            "EnemyMeteorSingle",
            handPos,
            EnemyConstants::BERSERK_BLAST_EFFECT_SCALE
        );

        blastEffectActive = true;
        hasTriggeredBlastEffect = true;
    }
}

void BerserkState::CheckAttackPartTransition(Enemy& enemy) {
    if (currentAttackSet == CharacterConstants::FIRST_ATTACK &&
        phaseTimer >= blast01To02Time) {
        TransitionToNextAttackPart(enemy);
    }
    else if (currentAttackSet == CharacterConstants::SECOND_ATTACK &&
        phaseTimer >= slash01To02Time) {
        TransitionToNextAttackPart(enemy);
    }
}

void BerserkState::HandleAttackAnimationEnd(Enemy& enemy) {
    // 攻撃が終わったら待機に戻る
    if (currentAttackPart == EnemyConstants::ATTACK_PART_TWO) {
        currentPhase = BerserkPhase::BERSERK_IDLE;
        phaseTimer = 0.0f;
        isTransitioning = false;

        enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::B_IDLE), true);

        // 次の攻撃までの待ち時間
        idleToAttackTime = attackCooldown;
    }
    // まだ途中なら次の段階へ
    else if (currentAttackPart == EnemyConstants::ATTACK_PART_ONE && !isTransitioning) {
        TransitionToNextAttackPart(enemy);
    }
}

void BerserkState::FacePlayer(Enemy& enemy) {

    // プレイヤーの方向を計算して向きを合わせる
    Player& player = Player::Instance();
    DirectX::XMFLOAT3 playerPos = player.GetPosition();

    DirectX::XMFLOAT3 direction = {
        playerPos.x - enemy.position.x,
        0.0f,
        playerPos.z - enemy.position.z
    };

    float length = sqrtf(direction.x * direction.x + direction.z * direction.z);

    if (length > 0.0f) {
        direction.x /= length;
        direction.z /= length;
    }

    float targetYaw = atan2f(direction.x, direction.z);

    enemy.angle.y = targetYaw;
}

void BerserkState::StartAttackSequence(Enemy& enemy) {

    // 攻撃用のフラグと状態をリセット
    hasTriggeredSlashEffect = false;
    hasTriggeredBlastEffect = false;
    hasSpawnedCollision = false;
    warningAreaPos = {};
    hasSpawnedSlashLightning = false;
    hasSpawnedWarningArea = false;
    hasSpawnedSlashProjectile = false;
    projectileDelayTimer = 0.0f;

    // 残っているエフェクトを止める
    if (blastEffectActive && blastEffectHandle != -1) {
        EffectManager::Instance().StopEffect(blastEffectHandle);
        blastEffectActive = false;
        blastEffectHandle = -1;
    }

    meteorImpactTime = 0.0f;

    // 攻撃セットに応じてアニメーションを選ぶ
    if (currentAttackSet == CharacterConstants::FIRST_ATTACK) {
        enemy.animationIndex = enemy.GetAnimIndex(EnemyAnim::METEOR_BLAST01);
    }
    else {
        enemy.animationIndex = enemy.GetAnimIndex(EnemyAnim::METEOR_SLASH01);
    }

    // アニメーションを初期化して再生
    enemy.animationLoop = false;
    enemy.animationSeconds = 0.0f;
    enemy.oldAnimationSeconds = 0.0f;
    enemy.model->PlayAnimation(enemy.animationIndex, false, enemy.animationBlendSeconds);

    // 攻撃実行状態に入る
    currentPhase = BerserkPhase::ATTACK_EXECUTION;
    phaseTimer = 0.0f;
    isTransitioning = false;
}

void BerserkState::TransitionToNextAttackPart(Enemy& enemy) {

    // 次の攻撃段階へ移行
    isTransitioning = true;
    currentAttackPart = EnemyConstants::ATTACK_PART_TWO;

    EnemyAnim nextAnim;

    // 攻撃セットごとに次のアニメーションを選ぶ
    if (currentAttackSet == CharacterConstants::FIRST_ATTACK) {
        nextAnim = EnemyAnim::METEOR_BLAST02;
    }
    else {
        nextAnim = EnemyAnim::METEOR_SLASH02;
    }

    // 次のアニメーションを再生
    enemy.StartAnimation(enemy.GetAnimIndex(nextAnim), false);

    // タイマーをリセット
    phaseTimer = 0.0f;
}

void BerserkState::UpdateBlastEffectPosition(Enemy& enemy) {

    // エフェクトが無ければ何もしない
    if (blastEffectHandle == -1) return;

    DirectX::XMFLOAT3 handPos = enemy.GetLeftHandPosition();
    DirectX::XMFLOAT3 enemyForward = enemy.GetForwardVector();

    // 手の前に少しオフセットした位置を計算
    DirectX::XMFLOAT3 offsetPos = {
        handPos.x + enemyForward.x * EnemyConstants::BERSERK_HAND_FORWARD_OFFSET,
        handPos.y,
        handPos.z + enemyForward.z * EnemyConstants::BERSERK_HAND_FORWARD_OFFSET
    };

    // エフェクトの位置を更新
    EffectManager::Instance().SetEffectPosition(blastEffectHandle, offsetPos);
}

bool BerserkState::IsPlayerInDangerZone() const
{
    Player& player = Player::Instance();
    DirectX::XMFLOAT3 playerPos = player.GetPosition();

    // 危険エリアの中心とサイズ
    DirectX::XMFLOAT3 cubeCenter = {
        EnemyConstants::DANGER_ZONE_CENTER_X,
        EnemyConstants::DANGER_ZONE_CENTER_Y,
        EnemyConstants::DANGER_ZONE_CENTER_Z
    };

    DirectX::XMFLOAT3 cubeSize = {
        EnemyConstants::DANGER_ZONE_SIZE_X,
        EnemyConstants::DANGER_ZONE_SIZE_Y,
        EnemyConstants::DANGER_ZONE_SIZE_Z
    };

    // 各軸で範囲内かチェック
    bool inX = (playerPos.x >= cubeCenter.x - cubeSize.x) &&
        (playerPos.x <= cubeCenter.x + cubeSize.x);

    bool inY = (playerPos.y >= cubeCenter.y - cubeSize.y) &&
        (playerPos.y <= cubeCenter.y + cubeSize.y);

    bool inZ = (playerPos.z >= cubeCenter.z - cubeSize.z) &&
        (playerPos.z <= cubeCenter.z + cubeSize.z);

    return inX && inY && inZ;
}

void BerserkState::ManageDangerZoneEffect(bool shouldBeActive, float elapsedTime) {

    Player& player = Player::Instance();

    if (shouldBeActive) {

        // すでにエフェクトが出ている場合は位置更新
        if (dangerZoneEffectActive) {

            DirectX::XMFLOAT3 playerPos = player.GetPosition();
            playerPos.y += EnemyConstants::BERSERK_TELEPORT_Y_OFFSET;

            if (dangerZoneEffectHandle != -1) {
                EffectManager::Instance().SetEffectPosition(dangerZoneEffectHandle, playerPos);
            }

            // 時間を進める
            dangerZoneEffectTimer += elapsedTime;

            // 一定時間後に攻撃を発生
            if (!hasTriggeredDangerHit && dangerZoneEffectTimer >= DANGER_HIT_FRAME_TIME) {

                DirectX::XMFLOAT3 projectilePos = player.GetPosition();

                EffectManager::Instance().PlayEffect(
                    "AreaMeteorBlast",
                    playerPos,
                    EnemyConstants::BERSERK_AREA_METEOR_SCALE
                );

                player.PlayPlayerSound(PlayerSound::HIT_BLAST, EnemyConstants::BERSERK_DANGER_ZONE_HIT_VOLUME);

                ProjectileManager::Instance().CreateProjectile(
                    "enemy",
                    "berserk_danger_zone_hit",
                    projectilePos,
                    { 0, 0, 0 }
                );

                hasTriggeredDangerHit = true;
            }
        }
        else {
            // 初回だけエフェクトを出す
            DirectX::XMFLOAT3 effectPos = player.GetPosition();
            effectPos.y += EnemyConstants::BERSERK_TELEPORT_Y_OFFSET;

            dangerZoneEffectHandle = EffectManager::Instance().PlayEffectWithHandle(
                "AreaMeteorStart",
                effectPos,
                EnemyConstants::BERSERK_AREA_METEOR_SCALE
            );

            Enemy::Instance().PlayEnemySound(
                EnemySound::AREA_WARNING,
                EnemyConstants::BERSERK_DANGER_ZONE_WARNING_VOLUME
            );

            dangerZoneEffectActive = true;
            dangerZoneEffectTimer = 0.0f;
        }
    }
    else if (dangerZoneEffectActive) {
        // エリア外に出たらエフェクト停止
        StopDangerZoneEffect();
        hasTriggeredDangerHit = false;
    }
}

void BerserkState::StopDangerZoneEffect() {

    // エフェクトが無ければ何もしない
    if (!dangerZoneEffectActive) return;

    if (dangerZoneEffectHandle != -1) {
        EffectManager::Instance().StopEffect(dangerZoneEffectHandle);
    }

    // 状態をリセット
    dangerZoneEffectActive = false;
    dangerZoneEffectHandle = -1;
    dangerZoneEffectTimer = 0.0f;
    hasTriggeredDangerHit = false;
}