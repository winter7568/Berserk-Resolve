#pragma once
#include "StateMachine/State.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameSystems/Effect/Effect.h"

// テレポート状態
class EnemyTeleportState : public State<Enemy, EnemyStateID>
{
public:
    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy, float elapsedTime) override;
    void Exit(Enemy& enemy) override;

    std::string GetName() const override { return "Teleport"; }
    EnemyStateID GetStateID() const override { return EnemyStateID::TELEPORT; }

    StatePriority GetPriority() const override { return StatePriority::MOVEMENT; }

    bool CanCombineWith(const State<Enemy, EnemyStateID>& other) const override {
        return other.GetPriority() > GetPriority();
    }

    static EnemyTeleportState& Instance() {
        static EnemyTeleportState instance;
        return instance;
    }

private:
    EnemyTeleportState() = default;

    // テレポートの段階
    enum class TeleportPhase {
        JUMP_START,
        INVISIBLE_LOOP,
        JUMP_LAND
    };

    TeleportPhase currentPhase = TeleportPhase::JUMP_START;

    float phaseTimer = 0.0f;

    DirectX::XMFLOAT3 teleportDestination = {};
    DirectX::XMFLOAT3 teleportStartPos = {};
    DirectX::XMFLOAT3 teleportEndPos = {};

    // 遠距離テレポートか（true: 遠距離, false: 近距離）
    bool isRangedTeleport = false;

    // テレポート先を決める
    void CalculateTeleportDestination();

    // プレイヤーの方向を向く
    void FacePlayer(Enemy& enemy);
};