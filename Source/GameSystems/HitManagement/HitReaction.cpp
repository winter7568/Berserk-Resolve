#include "HitManager.h"
#include "StateMachine/EnemyStates/HitStates.h"
#include "StateMachine/PlayerStates/PlayerHitState.h"
#include "GameSystems/Time/TimeManager.h"

// ============================================================================
// ヘルパー関数（ノックダウン判定）
// ============================================================================

bool HitManager::CheckAndApplyKnockdown(Enemy& enemy) {
    int currentHealth = enemy.GetHealth();
    int maxHealth = EnemyConstants::MAX_HEALTH;
    if (!enemy.HasBeenKnockedDown() && currentHealth <= (maxHealth * HitConstants::KNOCKDOWN_HEALTH_RATIO)) {
        enemy.SetKnockedDown(true);
        enemy.GetStateManager().PushState(&KnockDownState::Instance());
        return true;
    }
    return false;
}

// ============================================================================
// 敵リアクション
// ============================================================================

void HitManager::ApplyHitReaction(Enemy& enemy, const HitResult& hit) {
    enemy.ClearAttackAndMovementStates();
    enemy.ApplyDamage(hit.hitData.damage, HitConstants::DAMAGE_FALLBACK, hit.hitData);
    if (CheckAndApplyKnockdown(enemy)) {
        return;
    }
    switch (hit.hitData.hitType) {
    case Character::HitType::Regular:
        enemy.GetStateManager().PushState(&RegularHitState::Instance());
        break;
    case Character::HitType::Stun:
        enemy.GetStateManager().PushState(&StunState::Instance());
        break;
    case Character::HitType::Launch:
        enemy.GetStateManager().PushState(&LaunchHitState::Instance());
        break;
    default:
        enemy.GetStateManager().PushState(&RegularHitState::Instance());
        break;
    }
}

void HitManager::ApplyProjectileHitReaction(Enemy& enemy, const HitResult& hit, Projectile& projectile) {
    enemy.ClearAttackAndMovementStates();
    enemy.ApplyDamage(projectile.GetDamage(), HitConstants::DAMAGE_FALLBACK, hit.hitData);
    if (CheckAndApplyKnockdown(enemy)) {
        return;
    }
    if (projectile.GetImpactForce() > HitConstants::Enemy::PROJECTILE_FORCE_HITSTOP_THRESHOLD) {
        enemy.GetStateManager().PushState(&LaunchHitState::Instance());
    }
    else {
        enemy.GetStateManager().PushState(&RegularHitState::Instance());
    }
}

// 強制ヒット用：ダメージ適用とノックダウン・死亡判定
void HitManager::ApplyForceHitDamageAndKnockdown(Enemy& enemy, const HitResult& result) {
    enemy.ApplyDamage(result.hitData.damage, HitConstants::DAMAGE_FALLBACK, result.hitData);
    int currentHealth = enemy.GetHealth();
    int maxHealth = EnemyConstants::MAX_HEALTH;
    if (!enemy.HasBeenKnockedDown() && currentHealth <= (maxHealth * HitConstants::KNOCKDOWN_HEALTH_RATIO)) {
        enemy.ClearAttackAndMovementStates();
        enemy.SetKnockedDown(true);
        enemy.GetStateManager().PushState(&KnockDownState::Instance());
    }
    if (enemy.GetHealth() <= 0 && !enemy.GetStateManager().IsInState(EnemyStateID::DIE)) {
        enemy.GetStateManager().PushState(&EnemyDieState::Instance());
    }
}

// ============================================================================
// プレイヤーリアクション
// ============================================================================

void HitManager::ApplyPlayerHitReaction(Player& player, const HitResult& hit) {
    player.ClearAttackAndMovementStates();
    player.ApplyDamage(hit.hitData.damage, HitConstants::DAMAGE_FALLBACK, hit.hitData);
    switch (hit.hitData.hitType) {
    case Character::HitType::Regular:
        player.GetStateManager().PushState(&PlayerRegularHitState::Instance());
        break;
    case Character::HitType::Stun:
        player.GetStateManager().PushState(&PlayerStunState::Instance());
        break;
    case Character::HitType::Launch:
        player.GetStateManager().PushState(&PlayerLaunchHitState::Instance());
        break;
    }
}

void HitManager::ApplyEnemyProjectileHitReaction(Player& player, const HitResult& hit, Projectile& projectile) {
    player.ClearAttackAndMovementStates();
    player.ApplyDamage(projectile.GetDamage(), HitConstants::DAMAGE_FALLBACK, hit.hitData);
    switch (hit.hitData.hitType) {
    case Character::HitType::Regular:
        player.GetStateManager().PushState(&PlayerRegularHitState::Instance());
        break;
    case Character::HitType::Stun:
        player.GetStateManager().PushState(&PlayerStunState::Instance());
        break;
    case Character::HitType::Launch:
        player.GetStateManager().PushState(&PlayerLaunchHitState::Instance());
        break;
    }
}

// ============================================================================
// ブロック判定
// ============================================================================

bool HitManager::CheckEnemyBlock(Enemy& enemy, const DirectX::XMFLOAT3& hitDirection, float attackStrength) {
    if (enemy.GetStateManager().IsInState(EnemyStateID::BLOCK)) {
        Player& player = Player::Instance();
        DirectX::XMFLOAT3 enemyPos = enemy.GetPosition();
        DirectX::XMFLOAT3 playerPos = player.GetPosition();
        EnemyBlockState& blockState = EnemyBlockState::Instance();
        DirectX::XMFLOAT3 attackDirection = {
            playerPos.x - enemyPos.x,
            0.0f,
            playerPos.z - enemyPos.z
        };
        float attackLength = sqrt(attackDirection.x * attackDirection.x + attackDirection.z * attackDirection.z);
        if (attackLength > HitConstants::LENGTH_EPSILON) {
            attackDirection.x /= attackLength;
            attackDirection.z /= attackLength;
        }
        DirectX::XMFLOAT3 enemyForward = enemy.GetForwardVector();
        float dot = enemyForward.x * attackDirection.x + enemyForward.z * attackDirection.z;
        if (dot > 0.0f) {
            blockState.OnBlockSuccess(enemy, attackStrength);
            return true;
        }
    }
    return false;
}

// ============================================================================
// ヒットストップ
// ============================================================================

float HitManager::CalculateHitStopDuration(const HitResult& hit) const {
    float baseDuration = HitConstants::REGULAR_HIT_STOP_DURATION;
    switch (hit.hitData.hitType) {
    case Character::HitType::Regular:
        baseDuration = HitConstants::REGULAR_HIT_STOP_DURATION;
        break;
    case Character::HitType::Stun:
        baseDuration = HitConstants::STUN_HIT_STOP_DURATION;
        break;
    case Character::HitType::Launch:
        baseDuration = HitConstants::LAUNCH_HIT_STOP_DURATION;
        break;
    }
    return baseDuration;
}

void HitManager::ApplyHitStop(const HitResult& hit, bool isPlayerHit, float multiplier) {
    float hitStopDuration = CalculateHitStopDuration(hit) * multiplier;
    if (hit.hitData.hitType == Character::HitType::Regular) {
        hitStopDuration *= HitConstants::REGULAR_HIT_STOP_WEAKEN_FACTOR;
    }
    TimeManager::Instance().RequestHitStop(hitStopDuration);
}