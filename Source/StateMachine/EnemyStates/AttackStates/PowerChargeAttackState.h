#pragma once
#include "StateMachine/State.h"
#include "GameObjects/Enemy/Enemy.h"

// 溜め攻撃のステート
class EnemyPowerChargeState : public State<Enemy, EnemyStateID>
{
public:
    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy, float elapsedTime) override;
    void Exit(Enemy& enemy) override;

    std::string GetName() const override { return "PowerCharge"; }
    EnemyStateID GetStateID() const override { return EnemyStateID::POWER_CHARGE; }

    StatePriority GetPriority() const override { return StatePriority::ATTACK; }

    bool CanCombineWith(const State<Enemy, EnemyStateID>& other) const override
    {
        return other.GetPriority() > GetPriority();
    }

    static EnemyPowerChargeState& Instance()
    {
        static EnemyPowerChargeState instance;
        return instance;
    }

private:
    EnemyPowerChargeState() = default;

    // 溜め攻撃の進行状態
    enum class ChargePhase
    {
        CHARGE_START,
        CHARGE_HOLD,
        DOMAIN_SLASHES
    };

    ChargePhase currentPhase = ChargePhase::CHARGE_START;

    float phaseTimer = 0.0f;
    int slashesPerformed = 0;

    // 各スラッシュのヒット管理
    bool slashHit[6] = { false, false, false, false, false, false };

    // 位置を固定するために保存
    DirectX::XMFLOAT3 savedPosition = { 0.0f, 0.0f, 0.0f };
    bool positionLockedThisSequence = false;

    // チャージ開始位置（staticから移動）
    DirectX::XMFLOAT3 chargeStartPosition = { 0.0f, 0.0f, 0.0f };
    bool chargeStartPositionSaved = false;

    // プレイヤー方向に即座に向きを変える（補間なし）
    void InstantlyFacePlayer(Enemy& enemy);

    // フェーズごとの更新関数
    void UpdateChargeStart(Enemy& enemy);
    void UpdateChargeHold(Enemy& enemy, float elapsedTime);
    void UpdateDomainSlashes(Enemy& enemy);

    // ドメインスラッシュ用ヘルパー
    bool IsInSlashWindow(int currentFrame, int& outSlashIndex) const;
    void ProcessSlashHit(Enemy& enemy, int currentSlash);
    void FinishDomainSlashes(Enemy& enemy);
};