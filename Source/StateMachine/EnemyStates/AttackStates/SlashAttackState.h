#pragma once
#include "StateMachine/State.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameSystems/Effect/EffectManager.h"

// 通常攻撃のステート
class EnemyAttackState : public State<Enemy, EnemyStateID>
{
public:
    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy, float elapsedTime) override;
    void Exit(Enemy& enemy) override;

    std::string GetName() const override { return "Attack"; }
    EnemyStateID GetStateID() const override { return EnemyStateID::ATTACK; }

    StatePriority GetPriority() const override { return StatePriority::ATTACK; }

    // 自分より優先度が高いステートと同時に動く
    bool CanCombineWith(const State<Enemy, EnemyStateID>& other) const override
    {
        return other.GetPriority() > GetPriority();
    }

    // シングルトン
    static EnemyAttackState& Instance()
    {
        static EnemyAttackState instance;
        return instance;
    }

private:
    EnemyAttackState() = default;

    // 攻撃の進行状態
    enum class AttackPhase
    {
        RUN_START,
        RUN_LOOP,
        ATTACK
    };

    AttackPhase currentPhase = AttackPhase::RUN_START;

    // ループ制限変数
    int runLoopCount = 0;

    // 攻撃中の状態管理
    bool hasHitThisAttack = false;
    bool soundPlayedForThisAttack = false;

    Effekseer::Handle counterEffectHandle = -1;

    // カウンターウィンドウを更新する
    void UpdateCounterWindow(Enemy& enemy, int currentFrame);

    // ヒット検出を更新する
    void UpdateHitDetection(Enemy& enemy, int currentFrame);

    // アニメーション遷移を処理する
    void HandleAnimationTransition(Enemy& enemy);

    // RUN_LOOPフェーズの更新
    void UpdateRunLoop(Enemy& enemy);
};