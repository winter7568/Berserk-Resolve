#pragma once
#include <DirectXMath.h>
#include "StateMachine/State.h"
#include "GameObjects/Enemy/Enemy.h"

class Enemy;

// 待機状態のステート
class EnemyIdleState : public State<Enemy, EnemyStateID>
{
public:
    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy, float elapsedTime) override;
    void Exit(Enemy& enemy) override;

    std::string GetName() const override { return "Idle"; }
    EnemyStateID GetStateID() const override { return EnemyStateID::IDLE; }

    StatePriority GetPriority() const override { return StatePriority::MOVEMENT; }

    // 自分より優先度が高いステートと同時に動く
    bool CanCombineWith(const State<Enemy, EnemyStateID>& other) const override {
        return other.GetPriority() > GetPriority();
    }

    // シングルトン
    static EnemyIdleState& Instance() {
        static EnemyIdleState instance;
        return instance;
    }

private:
    EnemyIdleState() = default;

    float idleTimer = 0.0f;
};


// 歩き移動のステート
class EnemyWalkState : public State<Enemy, EnemyStateID>
{
public:
    static EnemyWalkState& Instance()
    {
        static EnemyWalkState instance;
        return instance;
    }

    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy, float elapsedTime) override;
    void Exit(Enemy& enemy) override;

    std::string GetName() const override { return "Walk"; }
    EnemyStateID GetStateID() const override { return EnemyStateID::WALK; }

    StatePriority GetPriority() const override { return StatePriority::MOVEMENT; }

    bool CanCombineWith(const State<Enemy, EnemyStateID>& other) const override {
        return other.GetPriority() > GetPriority();
    }

private:
    // 歩きの進行状態
    enum class WalkPhase
    {
        Start,
        Loop,
        Stop
    };

    WalkPhase currentPhase = WalkPhase::Start;
    float phaseTimer = 0.0f;
    float phaseDuration = 0.0f;
};


// ダッシュ移動のステート
class EnemyDashState : public State<Enemy, EnemyStateID>
{
public:
    static EnemyDashState& Instance()
    {
        static EnemyDashState instance;
        return instance;
    }

    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy, float elapsedTime) override;
    void Exit(Enemy& enemy) override;

    std::string GetName() const override { return "Dash"; }
    EnemyStateID GetStateID() const override { return EnemyStateID::DASH; }

    StatePriority GetPriority() const override { return StatePriority::MOVEMENT; }

    bool CanCombineWith(const State<Enemy, EnemyStateID>& other) const override {
        return other.GetPriority() > GetPriority();
    }

private:
    EnemyDashState() = default;

    float dashTimer = 0.0f;
};


// ガードのステート
class EnemyBlockState : public State<Enemy, EnemyStateID>
{
public:
    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy, float elapsedTime) override;
    void Exit(Enemy& enemy) override;

    std::string GetName() const override { return "Block"; }
    EnemyStateID GetStateID() const override { return EnemyStateID::BLOCK; }

    StatePriority GetPriority() const override { return StatePriority::REACTION; }

    // 他のステートを強制的に上書きする
    bool CanOverride(const State<Enemy, EnemyStateID>& other) const override { return true; }

    static EnemyBlockState& Instance() {
        static EnemyBlockState instance;
        return instance;
    }

    // 攻撃を防いだときに呼ばれる
    void OnBlockSuccess(Enemy& enemy, float attackStrength);

    // ガードが崩れたときに呼ばれる
    void OnBlockBreak(Enemy& enemy);

    // 今ガードするべきかを判定する
    static bool ShouldBlockNow(const Enemy& enemy, float elapsedTime);

private:
    EnemyBlockState() = default;

    // ガードの状態
    enum class BlockPhase {
        START,
        HOLD,
        HIT,
        RECOVERY,
        BREAK
    };

    BlockPhase currentPhase = BlockPhase::START;
    float phaseTimer = 0.0f;

    // ガード用のスタミナ
    float currentStamina = EnemyConstants::BLOCK_MAX_STAMINA;

    // ガードに関する設定
    struct BlockParams {
        float maxStamina = EnemyConstants::BLOCK_MAX_STAMINA;
        float staminaDrainLight = EnemyConstants::BLOCK_STAMINA_DRAIN_LIGHT;
        float staminaDrainHeavy = EnemyConstants::BLOCK_STAMINA_DRAIN_HEAVY;
        float staminaRegenRate = EnemyConstants::BLOCK_STAMINA_REGEN_RATE;
        float breakThreshold = EnemyConstants::BLOCK_BREAK_THRESHOLD;
        float startDuration = EnemyConstants::BLOCK_START_DURATION;
        float hitDuration = EnemyConstants::BLOCK_HIT_DURATION;
        float recoveryDuration = EnemyConstants::BLOCK_RECOVERY_DURATION;
        float breakDuration = EnemyConstants::BLOCK_BREAK_DURATION;
    } params;

    static float blockCooldownTimer;

    // ガード状態を更新する
    void UpdateBlockPhase(Enemy& enemy, float elapsedTime);

    // 状態を切り替える
    void ChangePhase(Enemy& enemy, BlockPhase newPhase);
};