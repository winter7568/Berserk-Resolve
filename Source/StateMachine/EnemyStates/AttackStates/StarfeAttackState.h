#pragma once
#include "StateMachine/State.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameSystems/Effect/EffectManager.h"
#include <vector>

// 横移動しながら攻撃するステート
class EnemyStarfeAttackState : public State<Enemy, EnemyStateID>
{
public:
    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy, float elapsedTime) override;
    void Exit(Enemy& enemy) override;

    std::string GetName() const override { return "StarfeAttack"; }
    EnemyStateID GetStateID() const override { return EnemyStateID::STARFE_ATTACK; }

    StatePriority GetPriority() const override { return StatePriority::ATTACK; }

    bool CanCombineWith(const State<Enemy, EnemyStateID>& other) const override
    {
        return other.GetPriority() > GetPriority();
    }

    static EnemyStarfeAttackState& Instance()
    {
        static EnemyStarfeAttackState instance;
        return instance;
    }

private:
    EnemyStarfeAttackState() = default;

    // 横移動攻撃の進行状態
    enum class StarfePhase
    {
        SIDESTEP_START,
        SIDESTEP_LOOP,
        PROJECTILE_FIRE,
        SIDESTEP_STOP
    };

    StarfePhase currentPhase = StarfePhase::SIDESTEP_START;

    float phaseTimer = 0.0f;
    float projectileTimer = 0.0f;

    int projectilesFired = 0;

    // 発射待ちの弾情報
    struct PendingProjectile
    {
        DirectX::XMFLOAT3 spawnPos;
        int index;
        float delayTimer;
        bool fired;
    };

    std::vector<PendingProjectile> pendingProjectiles;

    // 弾の生成位置を計算
    DirectX::XMFLOAT3 CalculateSpawnPosition(Enemy& enemy, int index);

    // 実際に弾を発射
    void FireProjectileNow(Enemy& enemy, const DirectX::XMFLOAT3& spawnPos, int index);

    // フェーズごとの更新関数
    void UpdateSideStepStart(Enemy& enemy);
    void UpdateSideStepLoop(Enemy& enemy);
    void UpdateProjectileFire(Enemy& enemy, float elapsedTime);
    void UpdateSideStepStop(Enemy& enemy)const;

    // 射撃用ヘルパー
    void FireProjectileWithSmoke(Enemy& enemy);
    void UpdatePendingProjectiles(Enemy& enemy, float elapsedTime);
    void CleanupFiredProjectiles();
    bool AllProjectilesFired() const;
};