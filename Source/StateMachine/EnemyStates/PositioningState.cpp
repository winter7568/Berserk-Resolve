#include "PositioningState.h"
#include "TeleportState.h"
#include "GameObjects/Player/Player.h"
#include "GameLib/TransformUtils.h"
#include "GameSystems/Projectile/ProjectileManager.h"
#include "StateMachine/EnemyStates/AttackStates/ComboAttackState.h"
#include "StateMachine/EnemyStates/AttackStates/StarfeAttackState.h"
#include "StateMachine/EnemyStates/AttackStates/SlashAttackState.h"
#include "StateMachine/EnemyStates/AttackStates/PowerChargeAttackState.h"

void EnemyPositioningState::Enter(Enemy& enemy) {
    stateEvaluationTimer = 0.0f;
    currentMovementStateID = EnemyStateID::NONE;
    projectileCount = 0;
    lastProjectileTime = 0.0f;
    projectileTimestamps.clear();
}

void EnemyPositioningState::Update(Enemy& enemy, float elapsedTime) {
    // 残り体力に応じて性格を変更する
    UpdatePersonalityBasedOnHealth(enemy.health, EnemyConstants::MAX_HEALTH);

    stateEvaluationTimer += elapsedTime;
    teleportCooldownTimer -= elapsedTime;

    auto& stateManager = enemy.GetStateManager();

    // プレイヤーの弾発射を監視する
    if (IsPlayerFiringProjectile()) {
        TrackProjectileEvent();
    }

    bool canTeleport = !stateManager.IsInState(EnemyStateID::TELEPORT);

    // 弾連打を検知したらテレポートからコンボ攻撃へ移行する
    if (IsPlayerSpammingProjectiles() &&
        !enemy.GetStateManager().IsInState(EnemyStateID::TELEPORT) &&
        !enemy.GetStateManager().IsInState(EnemyStateID::COMBO_ATTACK)) {

        TrackDecision(enemy, "ANTI-SPAM COMBO!");
        projectileCount = 0;
        enemy.GetStateManager().PushState(&EnemyTeleportState::Instance());
        return;
    }

    float distance = CalculateDistanceToPlayer(enemy);

    // 性格に基づいた不意打ちテレポート
    if (distance < EnemyConstants::POSITIONING_SURPRISE_ATTACK_DISTANCE &&
        !stateManager.IsInState(EnemyStateID::TELEPORT) &&
        !stateManager.IsInState(EnemyStateID::COMBO_ATTACK) &&
        enemy.ShouldTeleport()) {

        TrackDecision(enemy, "SURPRISE SIDE TELEPORT - Personality decision");
        stateManager.PushState(&EnemyTeleportState::Instance());
        return;
    }

    // 弾を避けるためにテレポートする
    if (ShouldTeleportToAvoidProjectile() && canTeleport) {
        if (stateManager.IsInState(EnemyStateID::REGULAR_HIT) ||
            stateManager.IsInState(EnemyStateID::LAUNCH_HIT)) {
            stateManager.RemoveState(EnemyStateID::REGULAR_HIT);
            stateManager.RemoveState(EnemyStateID::LAUNCH_HIT);
        }
        TrackDecision(enemy, "TELEPORT - Avoiding projectile!");
        TrackAction(enemy, "Teleport");
        teleportCooldownTimer = EnemyConstants::POSITIONING_TELEPORT_COOLDOWN;
        stateManager.PushState(&EnemyTeleportState::Instance());
        return;
    }

    // ブロックの機会を評価する
    bool didBlock = EvaluateBlockOpportunity(enemy, distance);
    if (didBlock) {
        return;
    }

    // 一定間隔で状態を再評価する
    if (stateEvaluationTimer >= EnemyConstants::POSITIONING_EVALUATION_INTERVAL) {
        ManageMovementStates(enemy, distance);
        stateEvaluationTimer = 0.0f;
    }
}

void EnemyPositioningState::ManageMovementStates(Enemy& enemy, float distance) {
    StateManager<Enemy, EnemyStateID>& stateManager = enemy.GetStateManager();

    Player& player = Player::Instance();
    static bool hasPunishedThisLaunch = false;

    // プレイヤーが打ち上げ状態になったら反撃する
    if (player.GetStateManager().IsInState(PlayerStateID::LAUNCH_HIT)) {
        if (!hasPunishedThisLaunch && !stateManager.IsInState(EnemyStateID::DASH)) {
            if (currentMovementStateID != EnemyStateID::NONE)
                stateManager.PushState(&EnemyDashState::Instance());
            currentMovementStateID = EnemyStateID::DASH;
            hasPunishedThisLaunch = true;
            return;
        }
    }
    else {
        hasPunishedThisLaunch = false;
    }

    // ブロック中は移動状態を変更しない
    if (stateManager.IsInState(EnemyStateID::BLOCK)) {
        return;
    }

    // 状態の整合性を確認する
    if (currentMovementStateID != EnemyStateID::NONE && !stateManager.IsInState(currentMovementStateID)) {
        currentMovementStateID = EnemyStateID::NONE;
    }

    float attackRange = GetAdjustedAttackRange();
    float walkStartDistance = GetAdjustedWalkRange();

    // 緊急バックダッシュ
    if (distance < distances.backDashDistance) {
        if (enemy.ShouldDash(distance)) {
            if (currentMovementStateID != EnemyStateID::DASH) {
                TrackDecision(enemy, "EMERGENCY BACKDASH - Personality decision");
                TrackAction(enemy, "Dash");
                if (currentMovementStateID != EnemyStateID::NONE)
                    stateManager.PopState();
                stateManager.PushState(&EnemyDashState::Instance());
                currentMovementStateID = EnemyStateID::DASH;
            }
            return;
        }
    }

    // 攻撃範囲内に入ったら攻撃を実行する
    if (distance <= attackRange) {

        bool isInAnyAttackState =
            stateManager.IsInState(EnemyStateID::ATTACK) ||
            stateManager.IsInState(EnemyStateID::COMBO_ATTACK) ||
            stateManager.IsInState(EnemyStateID::POWER_CHARGE) ||
            stateManager.IsInState(EnemyStateID::STARFE_ATTACK);

        bool shouldAttack = IsFacingPlayer(enemy);

        // 予測不能な行動 正面を向いていなくても攻撃することがある
        if (!shouldAttack && TransformUtils::RandomInt(0, EnemyConstants::POSITIONING_RANDOM_PERCENT_MAX - 1) < EnemyConstants::POSITIONING_UNPREDICTABLE_ATTACK_CHANCE) {
            shouldAttack = true;
        }

        if (shouldAttack && !isInAnyAttackState) {
            // 性格に基づいて特殊攻撃を使うか決める
            if (enemy.ShouldUseSpecialAttack()) {
                int specialChoice = TransformUtils::RandomInt(0, EnemyConstants::POSITIONING_RANDOM_PERCENT_MAX - 1);

                if (specialChoice < EnemyConstants::POSITIONING_SPECIAL_CHOICE_COMBO_THRESHOLD) {
                    stateManager.PushState(&EnemyComboAttackState::Instance());
                    TrackAction(enemy, "ComboAttack");
                }
                else if (specialChoice < EnemyConstants::POSITIONING_SPECIAL_CHOICE_POWER_THRESHOLD) {
                    stateManager.PushState(&EnemyPowerChargeState::Instance());
                    TrackAction(enemy, "PowerCharge");
                }
                else {
                    stateManager.PushState(&EnemyStarfeAttackState::Instance());
                    TrackAction(enemy, "StarfeAttack");
                }
            }
            else {
                // 通常攻撃でもたまに特殊攻撃を混ぜる
                if (TransformUtils::RandomInt(0, EnemyConstants::POSITIONING_RANDOM_PERCENT_MAX - 1) < EnemyConstants::POSITIONING_BASIC_MIXUP_CHANCE) {
                    stateManager.PushState(&EnemyPowerChargeState::Instance());
                    TrackAction(enemy, "SurprisePowerCharge");
                }
                else {
                    stateManager.PushState(&EnemyAttackState::Instance());
                    TrackAction(enemy, "BasicAttack");
                }
            }

            TrackDecision(enemy, "ATTACK - Aggressive decision");

            if (currentMovementStateID != EnemyStateID::NONE) {
                stateManager.PopState();
            }

            currentMovementStateID = EnemyStateID::ATTACK;
            return;
        }
    }

    // 遠すぎる場合はプレイヤーに向かって歩く
    if (distance > walkStartDistance) {
        if (currentMovementStateID != EnemyStateID::WALK) {
            TrackDecision(enemy, "WALK - Too far from player");
            TrackAction(enemy, "Walk");

            if (currentMovementStateID != EnemyStateID::NONE)
                stateManager.PopState();
            stateManager.PushState(&EnemyWalkState::Instance());
            currentMovementStateID = EnemyStateID::WALK;
        }
        return;
    }

    // 適切な距離に到達したら歩きを止める
    if (currentMovementStateID == EnemyStateID::WALK && distance <= distances.stopDistance) {
        TrackDecision(enemy, "STOP WALKING - Reached optimal range");
        TrackAction(enemy, "Idle");

        stateManager.PopState();
        stateManager.PushState(&EnemyIdleState::Instance());
        currentMovementStateID = EnemyStateID::IDLE;
        return;
    }

    // 快適な距離なら待機状態を維持する
    if (!stateManager.IsInState(EnemyStateID::IDLE) &&
        distance >= distances.backDashDistance &&
        distance <= walkStartDistance) {

        TrackDecision(enemy, "STAY IDLE - In comfortable range");
        TrackAction(enemy, "Idle");

        stateManager.PushState(&EnemyIdleState::Instance());
        currentMovementStateID = EnemyStateID::IDLE;
    }
}

float EnemyPositioningState::CalculateDistanceToPlayer(const Enemy& enemy) const {
    Player& player = Player::Instance();
    DirectX::XMFLOAT3 playerPos = player.GetPosition();
    DirectX::XMFLOAT3 enemyPos = enemy.GetPosition();

    float dx = playerPos.x - enemyPos.x;
    float dz = playerPos.z - enemyPos.z;
    return sqrtf(dx * dx + dz * dz);
}

void EnemyPositioningState::Exit(Enemy& enemy) {
    currentMovementStateID = EnemyStateID::NONE;
    stateEvaluationTimer = 0.0f;
    projectileCount = 0;
    lastProjectileTime = 0.0f;
    teleportCooldownTimer = 0.0f;
    projectileTimestamps.clear();
}

bool EnemyPositioningState::EvaluateBlockOpportunity(Enemy& enemy, float distance) {
    StateManager<Enemy, EnemyStateID>& stateManager = enemy.GetStateManager();

    if (stateManager.IsInState(EnemyStateID::BLOCK)) {
        return false;
    }

    bool isPlayerAttacking = Player::Instance().IsAttacking();
    if (enemy.ShouldBlock(distance, isPlayerAttacking)) {
        stateManager.PushState(&EnemyBlockState::Instance());
        return true;
    }

    return false;
}

bool EnemyPositioningState::IsFacingPlayer(const Enemy& enemy) {
    Player& player = Player::Instance();
    DirectX::XMFLOAT3 toPlayer = {
        player.GetPosition().x - enemy.GetPosition().x,
        0.0f,
        player.GetPosition().z - enemy.GetPosition().z
    };

    float length = sqrt(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);
    if (length > 0.001f) {
        toPlayer.x /= length;
        toPlayer.z /= length;
    }

    DirectX::XMFLOAT3 enemyForward = enemy.GetForwardVector();

    float dot = toPlayer.x * enemyForward.x + toPlayer.z * enemyForward.z;
    return dot > EnemyConstants::POSITIONING_FACING_DOT_THRESHOLD;
}

bool EnemyPositioningState::IsPlayerFiringProjectile() {
    auto& projectileManager = ProjectileManager::Instance();
    bool justFired = projectileManager.GetPlayerJustFiredProjectile();

    if (justFired) {
        projectileManager.ResetPlayerJustFiredProjectile();
        TrackProjectileEvent();
        return true;
    }

    return false;
}

bool EnemyPositioningState::ShouldTeleportToAvoidProjectile() {
    if (!IsPlayerFiringProjectile()) return false;

    return TransformUtils::RandomInt(0, EnemyConstants::POSITIONING_RANDOM_PERCENT_MAX - 1) < EnemyConstants::POSITIONING_PROJECTILE_TELEPORT_CHANCE;
}

float EnemyPositioningState::GetAdjustedAttackRange() const {
    return EnemyConstants::POSITIONING_BASE_ATTACK_RANGE * personality.attackRangeMultiplier;
}

float EnemyPositioningState::GetAdjustedWalkRange() const {
    return EnemyConstants::POSITIONING_BASE_WALK_RANGE * personality.walkRangeMultiplier;
}

void EnemyPositioningState::UpdatePersonalityBasedOnHealth(int currentHealth, int maxHealth) {
    float healthPercent = (float)currentHealth / (float)maxHealth;

    if (healthPercent < EnemyConstants::HEALTH_CRITICAL) {
        SetAggression(EnemyAggression::UNPREDICTABLE);
        SetTacticalStyle(TacticalStyle::MIXED);
    }
    else if (healthPercent < EnemyConstants::HEALTH_LOW) {
        SetAggression(EnemyAggression::PASSIVE);
        SetTacticalStyle(TacticalStyle::DEFENSIVE);
    }
    else if (healthPercent < EnemyConstants::HEALTH_MEDIUM) {
        SetAggression(EnemyAggression::BALANCED);
        SetTacticalStyle(TacticalStyle::STANDARD);
    }
    else {
        SetAggression(EnemyAggression::AGGRESSIVE);
        SetTacticalStyle(TacticalStyle::OFFENSIVE);
    }
}

bool EnemyPositioningState::IsPlayerSpammingProjectiles() {
    float currentTime = GetTickCount64() * 0.001f;

    if (currentTime - lastProjectileTime > EnemyConstants::POSITIONING_SPAM_DETECTION_WINDOW) {
        projectileCount = 0;
        return false;
    }

    return projectileCount >= EnemyConstants::POSITIONING_SPAM_THRESHOLD;
}

// 敵の判断記録を保存する
void EnemyPositioningState::TrackDecision(Enemy& enemy, const std::string& decision) {
    auto& debug = enemy.GetPositioningDebug();
    debug.currentDecision = decision;
    debug.decisionHistory.push_back(decision);
    if (debug.decisionHistory.size() > EnemyConstants::POSITIONING_DECISION_HISTORY_MAX) {
        debug.decisionHistory.erase(debug.decisionHistory.begin());
    }
}

// 敵の行動記録を保存する
void EnemyPositioningState::TrackAction(Enemy& enemy, const std::string& action) {
    enemy.GetPositioningDebug().lastAction = action;
}

void EnemyPositioningState::TrackProjectileEvent() {
    float currentTime = GetTickCount64() * 0.001f;

    if (currentTime - lastProjectileTime > EnemyConstants::POSITIONING_SPAM_DETECTION_WINDOW) {
        projectileCount = 0;
    }

    projectileCount++;
    lastProjectileTime = currentTime;
}

void EnemyPositioningState::UpdatePersonalitySettings() {

    switch (personality.aggression) {
    case EnemyAggression::PASSIVE:
        personality.attackRangeMultiplier = 0.6f;
        personality.walkRangeMultiplier = 1.3f;
        personality.optimalDistanceMultiplier = 1.3f;
        personality.blockChance = 0.95f;
        personality.counterChance = 0.5f;
        personality.specialAttackChance = 0.1f;
        personality.teleportChance = 0.02f;
        personality.dashChance = 0.95f;
        personality.decisionSpeed = 0.7f;
        personality.reactionSpeed = 0.8f;
        break;

    case EnemyAggression::BALANCED:
        personality.attackRangeMultiplier = 1.0f;
        personality.walkRangeMultiplier = 1.0f;
        personality.optimalDistanceMultiplier = 1.0f;
        personality.blockChance = 0.7f;
        personality.counterChance = 0.3f;
        personality.specialAttackChance = 0.4f;
        personality.teleportChance = 0.1f;
        personality.dashChance = 0.7f;
        personality.decisionSpeed = 1.0f;
        personality.reactionSpeed = 1.0f;
        break;

    case EnemyAggression::AGGRESSIVE:
        personality.attackRangeMultiplier = 1.4f;
        personality.walkRangeMultiplier = 0.6f;
        personality.optimalDistanceMultiplier = 0.7f;
        personality.blockChance = 0.4f;
        personality.counterChance = 0.1f;
        personality.specialAttackChance = 0.9f;
        personality.teleportChance = 0.35f;
        personality.dashChance = 0.3f;
        personality.decisionSpeed = 1.4f;
        personality.reactionSpeed = 1.3f;
        break;

    case EnemyAggression::UNPREDICTABLE:
        personality.attackRangeMultiplier = TransformUtils::RandomPositive(0.5f, 101, 0.01f);
        personality.blockChance = TransformUtils::RandomPositive(0.2f, 81, 0.01f);
        personality.counterChance = TransformUtils::RandomPositive(0.0f, 101, 0.01f);
        personality.specialAttackChance = TransformUtils::RandomPositive(0.1f, 91, 0.01f);
        personality.teleportChance = TransformUtils::RandomPositive(0.0f, 101, 0.01f);
        personality.dashChance = TransformUtils::RandomPositive(0.1f, 91, 0.01f);
        personality.decisionSpeed = TransformUtils::RandomPositive(0.5f, 101, 0.01f);        break;
    }

    switch (personality.style) {
    case TacticalStyle::DEFENSIVE:
        personality.blockChance += 0.15f;
        personality.counterChance += 0.15f;
        personality.dashChance += 0.15f;
        personality.specialAttackChance -= 0.25f;
        personality.teleportChance -= 0.05f;
        break;

    case TacticalStyle::OFFENSIVE:
        personality.blockChance -= 0.15f;
        personality.specialAttackChance += 0.25f;
        personality.teleportChance += 0.15f;
        personality.dashChance -= 0.15f;
        personality.counterChance -= 0.05f;
        break;

    case TacticalStyle::MIXED:
        static bool altBehavior = false;
        altBehavior = !altBehavior;
        if (altBehavior) {
            personality.blockChance += 0.25f;
            personality.specialAttackChance -= 0.25f;
            personality.dashChance += 0.2f;
        }
        else {
            personality.blockChance -= 0.25f;
            personality.specialAttackChance += 0.25f;
            personality.dashChance -= 0.2f;
        }
        break;
    }

    personality.blockChance = std::clamp(personality.blockChance, 0.0f, 1.0f);
    personality.counterChance = std::clamp(personality.counterChance, 0.0f, 1.0f);
    personality.specialAttackChance = std::clamp(personality.specialAttackChance, 0.0f, 1.0f);
    personality.teleportChance = std::clamp(personality.teleportChance, 0.0f, 1.0f);
    personality.dashChance = std::clamp(personality.dashChance, 0.0f, 1.0f);
}