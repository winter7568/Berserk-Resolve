#pragma once
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include "GameObjects/Player/Player.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameSystems/Projectile/Projectile.h"
#include "HitConstants.h"

struct HitResult {
    bool hitConnected = false;
    DirectX::XMFLOAT3 hitPoint = {};
    int attackIndex = 0;
    std::string attackType;
    Character::HitData hitData;
};

class HitManager {
public:
    static HitManager& Instance() {
        static HitManager instance;
        return instance;
    }
    HitManager(const HitManager&) = delete;
    HitManager& operator=(const HitManager&) = delete;

    bool ProcessWeaponHit(Player& player, Enemy& enemy);
    bool ProcessProjectileHit(Projectile& projectile, Enemy& enemy);
    bool ProcessEnemyWeaponHit(Enemy& enemy, Player& player);
    bool ProcessEnemyProjectileHit(Projectile& projectile, Player& player);

    void RegisterHit(const HitResult& result);
    const std::vector<HitResult>& GetRecentHits() const { return recentHits; }
    int GetComboCount() const { return currentCombo; }
    void ResetCombo() { currentCombo = 0; }

    void Reset() {
        recentHits.clear();
        currentCombo = 0;
        comboResetTimer = 0.0f;
    }

    void Update(float elapsedTime) {
        if (comboResetTimer > 0.0f) {
            comboResetTimer -= elapsedTime;
            if (comboResetTimer <= 0.0f) currentCombo = 0;
        }
    }

private:
    HitManager() = default;
    ~HitManager() = default;

    std::vector<HitResult> recentHits;
    int currentCombo = 0;
    float comboResetTimer = 0.0f;
    const float comboResetTime = 1.0f;

    // ヘルパー関数
    DirectX::XMFLOAT3 CalculateDirection(const DirectX::XMFLOAT3& from, const DirectX::XMFLOAT3& to);
    void ApplyForceHitDamageAndKnockdown(Enemy& enemy, const HitResult& result);
    bool ShouldForceHit(Enemy& enemy, Player& player);
    bool ShouldEnemyForceHit(Enemy& enemy, float hitForce);
    bool CheckAndApplyKnockdown(Enemy& enemy);

    // ヒットデータ作成
    Character::HitData CreatePlayerHitData(PlayerStateID state, int currentAttack, const DirectX::XMFLOAT3& hitPoint, const DirectX::XMFLOAT3& hitDirection);
    Character::HitData CreateEnemyHitData(EnemyStateID state, int currentAttack, int currentSlash, const DirectX::XMFLOAT3& hitPoint, const DirectX::XMFLOAT3& hitDirection);

    // リアクション処理
    void ApplyHitReaction(Enemy& enemy, const HitResult& hit);
    void ApplyProjectileHitReaction(Enemy& enemy, const HitResult& hit, Projectile& projectile);
    void ApplyPlayerHitReaction(Player& player, const HitResult& hit);
    void ApplyEnemyProjectileHitReaction(Player& player, const HitResult& hit, Projectile& projectile);

    // ガード判定
    bool CheckEnemyBlock(Enemy& enemy, const DirectX::XMFLOAT3& hitDirection, float attackStrength);

    // エフェクト
    void SpawnHitEffects(const HitResult& hit);
    void SpawnProjectileHitEffects(const HitResult& hit, Projectile& projectile);

    // ヒットストップ
    float CalculateHitStopDuration(const HitResult& hit) const;
    void ApplyHitStop(const HitResult& hit, bool isPlayerHit = false, float multiplier = 0.6f);
};