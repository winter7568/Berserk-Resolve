#pragma once
#include "StateMachine/State.h"
#include "GameObjects/Player/Player.h"

// 通常ヒットリアクション 小ダメージ
// 方向別ヒットアニメーションを再生し、短時間操作をロックする
class PlayerRegularHitState : public State<Player, PlayerStateID>
{
public:
    void Enter(Player& player) override;
    void Update(Player& player, float elapsedTime) override;
    void Exit(Player& player) override;

    std::string GetName() const override { return "RegularHit"; }
    PlayerStateID GetStateID() const override { return PlayerStateID::REGULAR_HIT; }

    // リアクション状態はほとんどのゲームプレイ状態を中断する
    StatePriority GetPriority() const override { return StatePriority::REACTION; }

    // リアクションタイマーが終了したら終了
    bool ShouldExit() const override { return timer <= 0.0f; }

    // ヒットリアクションはほとんど全ての状態を上書きできる
    bool CanOverride(const State<Player, PlayerStateID>& other) const override { return true; }

    static PlayerRegularHitState& Instance() {
        static PlayerRegularHitState instance;
        return instance;
    }

private:
    PlayerRegularHitState() = default;

    float timer = PlayerConstants::REGULAR_HIT_ANIM_DURATION;   // ヒットリアクションの継続時間
    bool wasInAir = false;  // 正しい状態に戻るために使用

    // プレイヤーに対するヒット方向を判定
    std::string CalculateHitDirection(const Player& player, const Character::HitData& hitData);

    // 方向別ヒットアニメーションを再生
    void PlayDirectionalHitAnimation(Player& player, const std::string& direction);
};

// 打ち上げヒット 大ダメージ
// 落下から起き上がりまでのフェーズを含む
class PlayerLaunchHitState : public State<Player, PlayerStateID>
{
public:
    void Enter(Player& player) override;
    void Update(Player& player, float elapsedTime) override;
    void Exit(Player& player) override;

    std::string GetName() const override { return "LaunchHit"; }
    PlayerStateID GetStateID() const override { return PlayerStateID::LAUNCH_HIT; }

    StatePriority GetPriority() const override { return StatePriority::REACTION; }

    // 最終フェーズが終了したら状態終了
    bool ShouldExit() const override { return currentPhase == LaunchPhase::Finished; }

    bool CanOverride(const State<Player, PlayerStateID>& other) const override { return true; }

    static PlayerLaunchHitState& Instance() {
        static PlayerLaunchHitState instance;
        return instance;
    }

private:
    PlayerLaunchHitState() = default;

    // 打ち上げ状態はフェーズで実行される
    enum class LaunchPhase {
        Falling,    // 空中ヒット 地面に落下中
        Launch,     // 地上ヒット 打ち上げアニメーション
        GetUp,      // 起き上がりアニメーション
        Finished    // 完了
    };

    LaunchPhase currentPhase = LaunchPhase::Launch;

    float phaseTimer = 0.0f;      // フェーズタイマー
    float phaseDuration = 0.0f;   // フェーズ継続時間

    bool wasInAir = false;  // ヒット時に空中にいたか
};

// プレイヤースタン状態
// 短時間移動も攻撃もできない
class PlayerStunState : public State<Player, PlayerStateID>
{
public:
    void Enter(Player& player) override;
    void Update(Player& player, float elapsedTime) override;
    void Exit(Player& player) override;

    std::string GetName() const override { return "Stun"; }
    PlayerStateID GetStateID() const override { return PlayerStateID::STUN; }

    StatePriority GetPriority() const override { return StatePriority::REACTION; }

    bool ShouldExit() const override { return stunTimer <= 0.0f; }

    bool CanOverride(const State<Player, PlayerStateID>& other) const override { return true; }

    static PlayerStunState& Instance() {
        static PlayerStunState instance;
        return instance;
    }

    // 状態に入る前にスタン時間を設定
    void SetStunDuration(float duration) { stunDuration = duration; }

private:
    PlayerStunState() = default;

    float stunTimer = 0.0f;       // スタン残り時間
    float stunDuration = PlayerConstants::STUN_DURATION;    // スタン継続時間
};

// プレイヤー死亡状態
// 死亡アニメーションを再生し、モデルをディゾルブする
class PlayerDeathState : public State<Player, PlayerStateID>
{
public:
    void Enter(Player& player) override;
    void Update(Player& player, float elapsedTime) override;
    void Exit(Player& player) override;

    std::string GetName() const override { return "Death"; }
    PlayerStateID GetStateID() const override { return PlayerStateID::DEATH; }

    // 最高優先度の状態
    StatePriority GetPriority() const override { return StatePriority::OVERRIDE; }

    bool ShouldExit() const override { return deathTimer <= 0.0f; }

    bool CanOverride(const State<Player, PlayerStateID>& other) const override { return true; }

    // 死亡状態は他の状態と結合できない
    bool CanCombineWith(const State<Player, PlayerStateID>& other) const override { return false; }

    static PlayerDeathState& Instance() {
        static PlayerDeathState instance;
        return instance;
    }

private:
    PlayerDeathState() = default;

    float deathTimer = 0.0f;      // 死亡タイマー
    float dissolveTimer = 0.0f;   // ディゾルブタイマー
};