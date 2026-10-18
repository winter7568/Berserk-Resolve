#include <cmath>
#include "HitManager.h"
#include "HitConstants.h"

// ヒット方向を計算
DirectX::XMFLOAT3 HitManager::CalculateDirection(const DirectX::XMFLOAT3& from, const DirectX::XMFLOAT3& to) {
    DirectX::XMFLOAT3 hitDirection = {
        from.x - to.x,
        0.0f,
        from.z - to.z
    };
    float length = sqrt(hitDirection.x * hitDirection.x + hitDirection.z * hitDirection.z);
    if (length > HitConstants::LENGTH_EPSILON) {
        hitDirection.x /= length;
        hitDirection.z /= length;
        return hitDirection;
    }
    else {
        // 方向が取れない場合はプレイヤー方向を使う
        DirectX::XMFLOAT3 fallback = {
            to.x - from.x,
            0.0f,
            to.z - from.z
        };
        float fallbackLength = sqrt(fallback.x * fallback.x + fallback.z * fallback.z);
        if (fallbackLength > HitConstants::LENGTH_EPSILON) {
            fallback.x /= fallbackLength;
            fallback.z /= fallbackLength;
            return fallback;
        }
        else {
            return { 0.0f, 0.0f, HitConstants::DEFAULT_HIT_DIRECTION_Z };
        }
    }
}

// 強制ヒット判定（武器攻撃用）
bool HitManager::ShouldForceHit(Enemy& enemy, Player& player) {
    return enemy.GetStateManager().IsInState(EnemyStateID::POWER_CHARGE) ||
        enemy.GetStateManager().IsInState(EnemyStateID::KNOCK_DOWN) ||
        enemy.GetStateManager().IsInState(EnemyStateID::TELEPORT) ||
        enemy.GetStateManager().IsInState(EnemyStateID::BERSERK) ||
        (enemy.GetStateManager().IsInState(EnemyStateID::ATTACK) &&
            !player.GetStateManager().IsInState(PlayerStateID::COUNTER)) ||
        (enemy.GetStateManager().IsInState(EnemyStateID::COMBO_ATTACK) &&
            !player.GetStateManager().IsInState(PlayerStateID::COUNTER));
}

// 強制ヒット判定（飛び道具用）
bool HitManager::ShouldEnemyForceHit(Enemy& enemy, float hitForce) {
    return enemy.GetStateManager().IsInState(EnemyStateID::KNOCK_DOWN) ||
        enemy.GetStateManager().IsInState(EnemyStateID::BERSERK) ||
        (enemy.GetStateManager().IsInState(EnemyStateID::ATTACK) &&
            hitForce <= HitConstants::Enemy::PROJECTILE_FORCE_HITSTOP_THRESHOLD);
}

Character::HitData HitManager::CreatePlayerHitData(
    PlayerStateID state,
    int currentAttack,
    const DirectX::XMFLOAT3& hitPoint,
    const DirectX::XMFLOAT3& hitDirection
) {
    Character::HitData hitData;
    hitData.hitOrigin = hitPoint;
    hitData.hitDirection = hitDirection;
    hitData.comboStep = currentAttack + HitConstants::COMBO_STEP_OFFSET;;

    if (state == PlayerStateID::ATTACK) {
        if (currentAttack == HitConstants::Player::SECOND_ATTACK_INDEX) {
            hitData.hitType = Character::HitType::Stun;
            hitData.hitForce = HitConstants::Player::STUN_HIT_FORCE;
            hitData.damage = HitConstants::Player::STUN_DAMAGE;
        }
        else if (currentAttack == HitConstants::Player::THIRD_ATTACK_INDEX) {
            hitData.hitType = Character::HitType::Launch;
            hitData.hitForce = HitConstants::Player::LAUNCH_HIT_FORCE;
            hitData.damage = HitConstants::Player::LAUNCH_DAMAGE;
        }
        else {
            hitData.hitType = Character::HitType::Regular;
            hitData.hitForce = HitConstants::Player::REGULAR_HIT_FORCE_BASE +
                (currentCombo * HitConstants::Player::COMBO_FORCE_MULTIPLIER);
            hitData.damage = HitConstants::Player::REGULAR_DAMAGE_BASE +
                (currentCombo * HitConstants::Player::COMBO_DAMAGE_MULTIPLIER);
        }
    }
    else if (state == PlayerStateID::AIR_ATTACK) {
        hitData.hitType = Character::HitType::Regular;
        hitData.hitForce = HitConstants::Player::AIR_ATTACK_HIT_FORCE;
        hitData.damage = HitConstants::Player::AIR_ATTACK_DAMAGE;
    }
    else if (state == PlayerStateID::PROJECTILE_ATTACK) {
        hitData.hitType = Character::HitType::Regular;
        hitData.hitForce = HitConstants::Player::PROJECTILE_HIT_FORCE;
        hitData.damage = HitConstants::Player::PROJECTILE_DAMAGE;
    }
    else if (state == PlayerStateID::COUNTER) {
        hitData.hitType = Character::HitType::Launch;
        hitData.hitForce = HitConstants::Player::COUNTER_HIT_FORCE;
        hitData.damage = HitConstants::Player::COUNTER_DAMAGE;
    }
    else {
        hitData.hitType = Character::HitType::Regular;
        hitData.hitForce = HitConstants::Player::DEFAULT_HIT_FORCE;
        hitData.damage = HitConstants::Player::DEFAULT_DAMAGE;
    }

    return hitData;
}

Character::HitData HitManager::CreateEnemyHitData(
    EnemyStateID state,
    int currentAttack,
    int currentSlash,
    const DirectX::XMFLOAT3& hitPoint,
    const DirectX::XMFLOAT3& hitDirection
) {
    Character::HitData hitData;
    hitData.hitOrigin = hitPoint;
    hitData.hitDirection = hitDirection;
    hitData.comboStep = currentAttack + HitConstants::COMBO_STEP_OFFSET;;

    if (state == EnemyStateID::POWER_CHARGE) {
        hitData.hitType = Character::HitType::Stun;
        hitData.hitForce = HitConstants::Enemy::POWER_CHARGE_HIT_FORCE;
        hitData.damage = HitConstants::Enemy::POWER_CHARGE_DAMAGE;
        hitData.comboStep = HitConstants::Enemy::DOMAIN_SLASH_COMBO_STEP;
    }
    else if (state == EnemyStateID::ATTACK) {
        hitData.hitType = Character::HitType::Stun;
        hitData.hitForce = HitConstants::Enemy::POWER_CHARGE_ATTACK_HIT_FORCE;
        hitData.damage = HitConstants::Enemy::POWER_CHARGE_ATTACK_DAMAGE;
    }
    else if (state == EnemyStateID::COMBO_ATTACK) {
        if (currentAttack == HitConstants::Player::SECOND_ATTACK_INDEX) {
            if (currentSlash == HitConstants::Enemy::FIRST_SLASH_INDEX) {
                hitData.hitType = Character::HitType::Regular;
                hitData.hitForce = HitConstants::Enemy::COMBO_ATTACK_HIT_FORCE_LIGHT;
                hitData.damage = HitConstants::Enemy::COMBO_ATTACK_DAMAGE_LIGHT;
            }
            else if (currentSlash == HitConstants::Enemy::SECOND_SLASH_INDEX) {
                hitData.hitType = Character::HitType::Stun;
                hitData.hitForce = HitConstants::Enemy::COMBO_ATTACK_HIT_FORCE_MEDIUM;
                hitData.damage = HitConstants::Enemy::COMBO_ATTACK_DAMAGE_MEDIUM;
            }
            else {
                hitData.hitType = Character::HitType::Regular;
                hitData.hitForce = HitConstants::Enemy::COMBO_ATTACK_HIT_FORCE_LIGHT;
                hitData.damage = HitConstants::Enemy::COMBO_ATTACK_DAMAGE_LIGHT;
            }
        }
        else if (currentAttack == HitConstants::Player::THIRD_ATTACK_INDEX) {
            hitData.hitType = Character::HitType::Launch;
            hitData.hitForce = HitConstants::Enemy::LAUNCH_HIT_FORCE;
            hitData.damage = HitConstants::Enemy::LAUNCH_DAMAGE;
        }
        else {
            hitData.hitType = Character::HitType::Regular;
            hitData.hitForce = HitConstants::Enemy::COMBO_ATTACK_HIT_FORCE_LIGHT;
            hitData.damage = HitConstants::Enemy::COMBO_ATTACK_DAMAGE_LIGHT;
        }
    }
    else {
        hitData.hitType = Character::HitType::Regular;
        hitData.hitForce = HitConstants::Enemy::DEFAULT_HIT_FORCE;
        hitData.damage = HitConstants::Enemy::DEFAULT_DAMAGE;
    }

    return hitData;
}