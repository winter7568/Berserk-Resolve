#pragma once
#include "StateMachine/State.h"
#include "GameObjects/Player/Player.h"
#include "AttackData.h"

// 近接攻撃状態
// プレイヤーの近接攻撃コンボを管理する
class PlayerAttackState : public State<Player, PlayerStateID>
{
public:
    void Enter(Player& player) override;
    void Update(Player& player, float elapsedTime) override;
    void Exit(Player& player) override;

    std::string GetName() const override { return "Attack"; }
    PlayerStateID GetStateID() const override { return PlayerStateID::ATTACK; }

    // 攻撃状態の優先度
    StatePriority GetPriority() const override { return StatePriority::ATTACK; }

    bool CanCombineWith(const State<Player, PlayerStateID>& other) const override {
        return other.GetPriority() > GetPriority();
    }

    static PlayerAttackState& Instance() {
        static PlayerAttackState instance;
        return instance;
    }

    // デバッグ用メソッド
    int GetCurrentAttackIndex() const { return currentAttack; }
    int GetCurrentFrame() const { return -1; }

private:
    PlayerAttackState() = default;

    // アニメーション名の配列
    std::vector<PlayerAnim> attackAnimations = {
        PlayerAnim::ATTACK_01_01,
        PlayerAnim::ATTACK_01_02,
        PlayerAnim::ATTACK_01_03,
        PlayerAnim::ATTACK_01_04
    };

    // フレームデータ 60fps想定
    const std::vector<AttackData> attackData = {
        {75, 8, 20, 25, 30, true},   // 攻撃1
        {88, 0, 20, 35, 40, true},   // 攻撃2
        {90, 5, 15, 15, 20, true},   // 攻撃3
        {97, 0, 50, -1, -1, false}   // 攻撃4 キャンセル不可
    };

    int currentAttack = 0;           // 現在の攻撃インデックス
    int queuedInputs = 0;            // キューされた入力数
    float inputBufferTime = 0.0f;    // 入力バッファ時間
    bool hasHitThisAttack = false;   // この攻撃でヒットしたか

    // ヒットウィンドウ追跡用
    bool isInHitWindow = false;      // ヒットウィンドウ内か
    int currentHitStart = 0;         // 現在のヒット開始フレーム
    int currentHitEnd = 0;           // 現在のヒット終了フレーム
    bool effectPlayedForSecondAttack = false;  // 2回目攻撃のエフェクトを再生済みか
    bool swingSoundPlayedForThisAttack = false; // この攻撃の振り音を再生済みか

    // ヒットウィンドウを更新
    void UpdateHitWindow(int currentFrame) {
        const AttackData& atk = attackData[currentAttack];
        isInHitWindow = (currentFrame >= atk.hitStart && currentFrame <= atk.hitEnd);
        currentHitStart = atk.hitStart;
        currentHitEnd = atk.hitEnd;
    }

    // 抽出したヘルパー関数
    bool CheckMoveOverride(Player& player);
    bool CheckInputOverrides(Player& player);
    void ProcessHit(Player& player);
    void CheckSecondAttackEffect(int currentFrame);
    void UpdateInputBuffer(float elapsedTime);
    bool CheckEarlyCancel(Player& player, int currentFrame);
    bool CheckCancelWindow(Player& player, int currentFrame);
    void FinishAttack(Player& player);
};