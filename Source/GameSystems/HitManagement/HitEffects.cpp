#include "HitManager.h"
#include "GameSystems/Effect/EffectManager.h"
#include "GameSystems/Particle/ParticleManager.h"

void HitManager::SpawnHitEffects(const HitResult& hit)
{
    EffectManager::Instance().PlayEffect("playerHit", hit.hitPoint, HitConstants::EFFECT_SCALE);
}

void HitManager::SpawnProjectileHitEffects(const HitResult& hit, Projectile& projectile)
{
    std::string effectType = projectile.GetHitEffect();
    Enemy& enemy = Enemy::Instance();
    DirectX::XMFLOAT3 enemyPos = enemy.GetPosition();

    if (effectType == "explosion_hit") {
        ParticleManager::Instance().SpawnLightningImpact(
            hit.hitPoint,
            hit.hitData.hitDirection,
            HitConstants::Enemy::LIGHTNING_IMPACT_SCALE
        );
    }
    else if (effectType == "spark_hit") {
        EffectManager::Instance().PlayEffect("playerProjectile", hit.hitPoint, HitConstants::EFFECT_SCALE);
    }
    else if (effectType == "enemy_spark_hit") {
        EffectManager::Instance().PlayEffect("playerProjHit", hit.hitPoint, HitConstants::EFFECT_SCALE);
    }
    else if (effectType == "magic_hit" || effectType == "arrow_hit") {
        EffectManager::Instance().PlayEffect("enemyProj2Hit", hit.hitPoint, HitConstants::EFFECT_SCALE);
    }
    else if (effectType == "slash_hit") {
        EffectManager::Instance().PlayEffect("enemyAirProj1_2", hit.hitPoint, HitConstants::EFFECT_SCALE);
    }
    else if (effectType == "beam_hit") {
        EffectManager::Instance().PlayEffect("enemyAirProj4", enemyPos, HitConstants::EFFECT_SCALE);
    }
}