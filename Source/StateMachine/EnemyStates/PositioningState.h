#pragma once
#include <DirectXMath.h>
#include <algorithm>
#include <deque>

#include "StateMachine/State.h"
#include "GameObjects/Enemy/Enemy.h"
#include "MoveStates.h"


class EnemyPositioningState : public State<Enemy, EnemyStateID>
{
public:
    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy, float elapsedTime) override;
    void Exit(Enemy& enemy) override;

    std::string GetName() const override { return "Positioning"; }
    EnemyStateID GetStateID() const override { return EnemyStateID::POSITIONING; }
    StatePriority GetPriority() const override { return StatePriority::BRAIN; }

    // 自分より優先度が高いステートとは同時に動く
    bool CanCombineWith(const State<Enemy, EnemyStateID>& other) const override {
        return other.GetPriority() > GetPriority();
    }

    static EnemyPositioningState& Instance() {
        static EnemyPositioningState instance;
        return instance;
    }

    // 敵の攻撃性のタイプ
    enum class EnemyAggression {
        PASSIVE,
        BALANCED,
        AGGRESSIVE,
        UNPREDICTABLE
    };

    // 戦い方のスタイル
    enum class TacticalStyle {
        DEFENSIVE,
        STANDARD,
        OFFENSIVE,
        MIXED
    };

    float GetBlockChance() const { return personality.blockChance; }
    float GetDashChance() const { return personality.dashChance; }
    float GetCounterChance() const { return personality.counterChance; }
    float GetSpecialAttackChance() const { return personality.specialAttackChance; }
    float GetTeleportChance() const { return personality.teleportChance; }

    // 性格によって調整された距離を返す
    float GetAdjustedAttackRange() const;
    float GetAdjustedWalkRange() const;

    void SetAggression(EnemyAggression newAggression) {
        personality.aggression = newAggression;
        UpdatePersonalitySettings();
    }

    void SetTacticalStyle(TacticalStyle style) {
        personality.style = style;
        UpdatePersonalitySettings();
    }

    // 値が変な範囲にならないように制限する
    void SetBlockChance(float chance) { personality.blockChance = std::clamp(chance, 0.0f, 1.0f); }
    void SetDashChance(float chance) { personality.dashChance = std::clamp(chance, 0.0f, 1.0f); }
    void SetCounterChance(float chance) { personality.counterChance = std::clamp(chance, 0.0f, 1.0f); }
    void SetSpecialAttackChance(float chance) { personality.specialAttackChance = std::clamp(chance, 0.0f, 1.0f); }
    void SetTeleportChance(float chance) { personality.teleportChance = std::clamp(chance, 0.0f, 1.0f); }

    // 体力に応じて性格を変える
    void UpdatePersonalityBasedOnHealth(int currentHealth, int maxHealth);

private:
    EnemyPositioningState() = default;

    // 行動の確率や距離などの設定
    struct PersonalitySettings {
        EnemyAggression aggression = EnemyAggression::BALANCED;
        TacticalStyle style = TacticalStyle::STANDARD;

        float blockChance = 0.8f;
        float counterChance = 0.3f;
        float specialAttackChance = 0.5f;
        float teleportChance = 0.15f;
        float dashChance = 0.7f;

        float attackRangeMultiplier = 1.0f;
        float walkRangeMultiplier = 1.0f;
        float optimalDistanceMultiplier = 1.0f;

        float decisionSpeed = 1.0f;
        float reactionSpeed = 1.0f;
    };

    // 距離に関する設定
    struct DistanceParams {
        float backDashDistance = EnemyConstants::POSITIONING_BACK_DASH_DISTANCE;
        float walkStartDistance = EnemyConstants::POSITIONING_BASE_WALK_RANGE;
        float optimalDistance = EnemyConstants::POSITIONING_OPTIMAL_DISTANCE;
        float stopDistance = EnemyConstants::POSITIONING_STOP_DISTANCE;
    } distances;

    PersonalitySettings personality;

    // 距離に応じて移動ステートを切り替える
    void ManageMovementStates(Enemy& enemy, float currentDistance);

    // プレイヤーとの距離を計算する
    float CalculateDistanceToPlayer(const Enemy& enemy) const;

    float stateEvaluationTimer = 0.0f;

    // プレイヤーの方向を向いているか確認する
    bool IsFacingPlayer(const Enemy& enemy);

    // ガードするかどうかを判断する
    bool EvaluateBlockOpportunity(Enemy& enemy, float distance);

    bool IsPlayerFiringProjectile();

    // 飛び道具を避けるためにテレポートするか判断する
    bool ShouldTeleportToAvoidProjectile();

    float teleportCooldownTimer = 0.0f;
    float lastProjectileTime = 0.0f;
    int projectileCount = 0;

    EnemyStateID currentMovementStateID = EnemyStateID::NONE;

    // 性格設定を更新する
    void UpdatePersonalitySettings();

    // 飛び道具の発射タイミングを記録する
    std::deque<float> projectileTimestamps;

    // 連続で飛び道具を使っているか確認する
    bool IsPlayerSpammingProjectiles();

    // 敵の判断記録を保存する
    static void TrackDecision(Enemy& enemy, const std::string& decision);
    // 敵の行動記録を保存する
    static void TrackAction(Enemy& enemy, const std::string& action);

    void TrackProjectileEvent();
};