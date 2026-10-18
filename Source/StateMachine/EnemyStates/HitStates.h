#pragma once
#include <DirectXMath.h>

#include "StateMachine/State.h"
#include "GameObjects/Enemy/Enemy.h"
#include "ShaderSystems/FadeManager.h"

class Enemy;

// 通常ヒット時のステート
class RegularHitState : public State<Enemy, EnemyStateID>
{
public:
    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy, float elapsedTime) override;
    void Exit(Enemy& enemy) override;

    std::string GetName() const override { return "RegularHit"; }
    EnemyStateID GetStateID() const override { return EnemyStateID::REGULAR_HIT; }

    StatePriority GetPriority() const override { return StatePriority::REACTION; }

    // タイマーが終わったら抜ける
    bool ShouldExit() const override { return timer <= 0.0f; }

    // 他のステートを上書きする
    bool CanOverride(const State<Enemy, EnemyStateID>& other) const override { return true; }

    static RegularHitState& Instance() {
        static RegularHitState instance;
        return instance;
    }

private:
    RegularHitState() = default;

    // ヒットリアクションの時間
    float timer = EnemyConstants::REGULAR_HIT_ANIM_DURATION;

    // 攻撃がどの方向から来たかを計算
    std::string CalculateHitDirection(const Enemy& enemy, const Character::HitData& hitData);

    // 方向に応じたヒットアニメーションを再生
    void PlayDirectionalHitAnimation(Enemy& enemy, const std::string& direction);
};


// 打ち上げヒットのステート
class LaunchHitState : public State<Enemy, EnemyStateID>
{
public:
    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy, float elapsedTime) override;
    void Exit(Enemy& enemy) override;

    std::string GetName() const override { return "LaunchHit"; }
    EnemyStateID GetStateID() const override { return EnemyStateID::LAUNCH_HIT; }

    StatePriority GetPriority() const override { return StatePriority::REACTION; }

    // フェーズが終わったら抜ける
    bool ShouldExit() const override { return currentPhase == LaunchPhase::Finished; }

    bool CanOverride(const State<Enemy, EnemyStateID>& other) const override { return true; }

    static LaunchHitState& Instance() {
        static LaunchHitState instance;
        return instance;
    }

private:
    LaunchHitState() = default;

    // 打ち上げの進行状態
    enum class LaunchPhase {
        Launch,
        Finished
    };

    LaunchPhase currentPhase = LaunchPhase::Launch;

    float phaseTimer = 0.0f;
    float phaseDuration = 0.0f;
};


// スタン状態のステート
class StunState : public State<Enemy, EnemyStateID>
{
public:
    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy, float elapsedTime) override;
    void Exit(Enemy& enemy) override;

    std::string GetName() const override { return "Stun"; }
    EnemyStateID GetStateID() const override { return EnemyStateID::STUN; }

    StatePriority GetPriority() const override { return StatePriority::OVERRIDE; }

    // スタン時間が終わったら抜ける
    bool ShouldExit() const override { return stunTimer <= 0.0f; }

    bool CanOverride(const State<Enemy, EnemyStateID>& other) const override { return true; }

    static StunState& Instance() {
        static StunState instance;
        return instance;
    }

    // スタン時間を外から設定する
    void SetStunDuration(float duration) { stunDuration = duration; }

private:
    StunState() = default;

    float stunTimer = 0.0f;

    // スタンの長さ
    float stunDuration = EnemyConstants::STUN_DURATION_DEFAULT;
};


// ダウン状態のステート
class KnockDownState : public State<Enemy, EnemyStateID>
{
public:
    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy, float elapsedTime) override;
    void Exit(Enemy& enemy) override;

    std::string GetName() const override { return "KnockDown"; }
    EnemyStateID GetStateID() const override { return EnemyStateID::KNOCK_DOWN; }

    StatePriority GetPriority() const override { return StatePriority::REACTION; }

    // フェーズが終わったら抜ける
    bool ShouldExit() const override { return currentPhase == KnockDownPhase::Finished; }

    bool CanOverride(const State<Enemy, EnemyStateID>& other) const override { return true; }

    static KnockDownState& Instance() {
        static KnockDownState instance;
        return instance;
    }

private:
    KnockDownState() = default;

    // ダウンの進行状態
    enum class KnockDownPhase {
        Start,
        Loop,
        End,
        Finished
    };

    KnockDownPhase currentPhase = KnockDownPhase::Start;

    float phaseTimer = 0.0f;
    float phaseDuration = 0.0f;

    // 各フェーズの時間
    const float startDuration = EnemyConstants::KNOCKDOWN_START_DURATION;
    const float loopDuration = EnemyConstants::KNOCKDOWN_LOOP_DURATION;
    const float endDuration = EnemyConstants::KNOCKDOWN_END_DURATION;
};


// 死亡ステート
class EnemyDieState : public State<Enemy, EnemyStateID>
{
public:
    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy, float elapsedTime) override;
    void Exit(Enemy& enemy) override;

    std::string GetName() const override { return "Die"; }
    EnemyStateID GetStateID() const override { return EnemyStateID::DIE; }

    StatePriority GetPriority() const override { return StatePriority::OVERRIDE; }

    bool CanOverride(const State<Enemy, EnemyStateID>& other) const override { return true; }

    // 他のステートと同時には動かない
    bool CanCombineWith(const State<Enemy, EnemyStateID>& other) const override { return false; }

    static EnemyDieState& Instance() {
        static EnemyDieState instance;
        return instance;
    }

private:
    EnemyDieState() = default;

    float dieTimer = 0.0f;

    // 消える演出の時間
    float dissolveTimer = 0.0f;
};