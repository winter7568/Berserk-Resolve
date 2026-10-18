#pragma once
#include <DirectXMath.h>

#include "StateMachine/State.h"
#include "GameObjects/Player/Player.h"

class PlayerIdleState : public State<Player, PlayerStateID>
{
public:
    void Enter(Player& player) override;
    void Update(Player& player, float elapsedTime) override;
    void Exit(Player& player) override;

    std::string GetName() const override { return "Idle"; }
    PlayerStateID GetStateID() const override { return PlayerStateID::IDLE; }

    // 移動状態の優先度
    StatePriority GetPriority() const override { return StatePriority::MOVEMENT; }

    bool CanCombineWith(const State<Player, PlayerStateID>& other) const override {
        return other.GetPriority() > GetPriority();
    }

    static PlayerIdleState& Instance() {
        static PlayerIdleState instance;
        return instance;
    }

private:
    PlayerIdleState() = default;
};


class PlayerRunState : public State<Player, PlayerStateID>
{
public:
    void Enter(Player& player) override;
    void Update(Player& player, float elapsedTime) override;
    void Exit(Player& player) override;

    std::string GetName() const override { return "Run"; }
    PlayerStateID GetStateID() const override { return PlayerStateID::RUN; }

    // 移動状態の優先度
    StatePriority GetPriority() const override { return StatePriority::MOVEMENT; }

    bool CanCombineWith(const State<Player, PlayerStateID>& other) const override {
        return other.GetPriority() > GetPriority();
    }

    static PlayerRunState& Instance() {
        static PlayerRunState instance;
        return instance;
    }

    float GetMotionScale() const { return motionScale; }
private:
    // ランのフェーズ
    enum class RunPhase { Start, Loop, Stop };
    RunPhase currentPhase = RunPhase::Start;  // 現在のフェーズ

    float motionScale = PlayerConstants::RUN_START_MOTION_SCALE;   // モーション速度倍率
    float phaseTimer = 0.0f;      // フェーズタイマー
    float phaseDuration = 0.0f;   // フェーズ継続時間
};


class PlayerJumpState : public State<Player, PlayerStateID>
{
public:
    void Enter(Player& player) override;
    void Update(Player& player, float elapsedTime) override;
    void Exit(Player& player) override;

    std::string GetName() const override { return "Jump"; }
    PlayerStateID GetStateID() const override { return PlayerStateID::JUMP; }

    // アクション状態の優先度
    StatePriority GetPriority() const override { return StatePriority::ACTION; }

    bool CanCombineWith(const State<Player, PlayerStateID>& other) const override {
        return other.GetPriority() > GetPriority();
    }

    static PlayerJumpState& Instance() {
        static PlayerJumpState instance;
        return instance;
    }

private:
    // ジャンプのフェーズ
    enum class JumpPhase { Start, Loop, Land };
    JumpPhase currentPhase = JumpPhase::Start;  // 現在のフェーズ

    float phaseTimer = 0.0f;      // フェーズタイマー
    float phaseDuration = 0.0f;   // フェーズ継続時間

    bool canDoubleJump = false;    // ダブルジャンプ可能か
    bool hasDoubleJumped = false;  // ダブルジャンプ済みか

    void ApplyAirPhysics(Player& player, float elapsedTime);           // 空中物理演算を適用
};


class PlayerDashState : public State<Player, PlayerStateID>
{
public:
    void Enter(Player& player) override;
    void Update(Player& player, float elapsedTime) override;
    void Exit(Player& player) override;

    std::string GetName() const override { return "Dash"; }
    PlayerStateID GetStateID() const override { return PlayerStateID::DASH; }

    // アクション状態の優先度
    StatePriority GetPriority() const override { return StatePriority::ACTION; }

    bool CanCombineWith(const State<Player, PlayerStateID>& other) const override {
        return other.GetPriority() < GetPriority();
    }

    static PlayerDashState& Instance() {
        static PlayerDashState instance;
        return instance;
    }

private:
    PlayerDashState() = default;

    bool wasInAir = false;             // 空中でダッシュを開始したか
    float initialAirY = 0.0f;          // 空中ダッシュ開始時のY座標
    float initialVerticalVelocity = 0.0f;  // 空中ダッシュ開始時の垂直速度
    float dashDuration = 0.0f;         // ダッシュ継続時間
};


class PlayerFallingState : public State<Player, PlayerStateID>
{
public:
    void Enter(Player& player) override;
    void Update(Player& player, float elapsedTime) override;
    void Exit(Player& player) override;

    std::string GetName() const override { return "Falling"; }
    PlayerStateID GetStateID() const override { return PlayerStateID::FALLING; }

    // アクション状態の優先度
    StatePriority GetPriority() const override { return StatePriority::ACTION; }

    bool CanCombineWith(const State<Player, PlayerStateID>& other) const override {
        return other.GetPriority() > GetPriority();
    }

    static PlayerFallingState& Instance() {
        static PlayerFallingState instance;
        return instance;
    }

private:
    // 落下のフェーズ
    enum class FallPhase { Loop, Land };
    FallPhase currentPhase = FallPhase::Loop;  // 現在のフェーズ

    float phaseTimer = 0.0f;      // フェーズタイマー
    float phaseDuration = 0.0f;   // フェーズ継続時間

    void ApplyFallingPhysics(Player& player, float elapsedTime);        // 落下物理演算を適用
};