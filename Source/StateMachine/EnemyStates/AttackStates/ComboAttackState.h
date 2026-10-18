#pragma once
#include "StateMachine/State.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameSystems/Effect/EffectManager.h"
#include <vector>

// コンボ攻撃のステート
class EnemyComboAttackState : public State<Enemy, EnemyStateID>
{
public:
    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy, float elapsedTime) override;
    void Exit(Enemy& enemy) override;

    std::string GetName() const override { return "ComboAttack"; }
    EnemyStateID GetStateID() const override { return EnemyStateID::COMBO_ATTACK; }

    StatePriority GetPriority() const override { return StatePriority::ATTACK; }

    bool CanCombineWith(const State<Enemy, EnemyStateID>& other) const override
    {
        return other.GetPriority() > GetPriority();
    }

    static EnemyComboAttackState& Instance()
    {
        static EnemyComboAttackState instance;
        return instance;
    }

private:
    EnemyComboAttackState() = default;

    // 各攻撃のアニメーション
    std::vector<EnemyAnim> attackAnimations = {
        EnemyAnim::COMBO_ATTACK_01,
        EnemyAnim::COMBO_ATTACK_02,
        EnemyAnim::COMBO_ATTACK_03,
        EnemyAnim::COMBO_ATTACK_04
    };

    // フレーム単位の攻撃タイミング
    struct AttackTiming
    {
        int totalFrames;
        int transitionFrame;
        int hitStartFrame;
        int hitEndFrame;
    };

    const std::vector<AttackTiming> attackTimings = {
        {95, 26, 7, 15},
        {84, 26, 5, 15},
        {115, 50, 10, 50},
        {123, -1, 40, 50}
    };

    int currentAttack = 0;
    int currentFrame = 0;
    bool hasHitThisAttack = false;
    int currentSlash = 0;

    Effekseer::Handle counterEffectHandle = -1;

    // 次の攻撃に移行
    void TransitionToNextAttack(Enemy& enemy);

    // カウンターウィンドウを更新する
    void UpdateCounterWindow(Enemy& enemy);

    // 発光色をパルスさせる
    void UpdateEmissivePulse(Enemy& enemy, float elapsedTime);

    // ヒット検出を更新する
    void UpdateHitDetection(Enemy& enemy);

    // 3回目の攻撃のヒット検出
    void UpdateHitDetectionForThirdAttack(Enemy& enemy);

    // 他の攻撃のヒット検出
    void UpdateHitDetectionForOtherAttacks(Enemy& enemy);

    // 次の攻撃へ移行するか判定する
    void CheckTransition(Enemy& enemy);

    // アニメーション終了時の処理
    void CheckAttackCompletion(Enemy& enemy);
};