#pragma once
#include "StateMachine/State.h"
#include "GameObjects/Player/Player.h"

// カウンターフェーズ
enum class CounterPhase
{
    None,           // なし
    JumpBack,       // ジャンプバック中
    JumpBackEnd,    // ジャンプバック終了
    CounterAttack   // カウンター攻撃中
};

// プレイヤーカウンター状態
// 敵の攻撃をカウンターする
class PlayerCounterState : public State<Player, PlayerStateID>
{
public:
    void Enter(Player& player) override;
    void Update(Player& player, float elapsedTime) override;
    void Exit(Player& player) override;

    std::string GetName() const override { return "Counter"; }
    PlayerStateID GetStateID() const override { return PlayerStateID::COUNTER; }

    // リアクション状態はほとんどの状態を中断する
    StatePriority GetPriority() const override { return StatePriority::REACTION; }

    // フェーズがなしになったら終了
    bool ShouldExit() const override { return phase == CounterPhase::None; }

    // 他の状態を上書きできる
    bool CanOverride(const State<Player, PlayerStateID>& other) const override { return true; }

    static PlayerCounterState& Instance() {
        static PlayerCounterState instance;
        return instance;
    }

    // 敵の攻撃をカウンターできるかチェック
    bool CanCounterEnemyAttack(const Player& player) const;

private:
    PlayerCounterState() = default;

    CounterPhase phase = CounterPhase::None;  // 現在のフェーズ

    // ジャンプバック移動用
    DirectX::XMFLOAT3 backwardDirection = { 0.0f, 0.0f, 0.0f };  // 後退方向
    float jumpBackVelocity = 0.0f;   // ジャンプバック速度
    float jumpBackTimer = 0.0f;       // ジャンプバックタイマー
    float jumpBackDuration = 0.0f;    // ジャンプバック継続時間

    bool queuedCounter = false;   // ジャンプバック中に左クリックが押されたか
    bool hitRegistered = false;   // 二重ヒットを防ぐ
    bool wasComboCounter = false; // コンボ攻撃のカウンターだったか

    // 敵がカウンターウィンドウ内かチェック
    bool IsEnemyInCounterWindow() const;

    // 後退方向を計算 敵から遠ざかる方向
    void CalculateBackwardDirection(Player& player);
};