#pragma once
#include "StateMachine/State.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameObjects/Player/Player.h"
#include "GameSystems/Particle/ParticleSprite.h"
#include "GameSystems/Time/TimeManager.h"

// バーサク状態のステート
class BerserkState : public State<Enemy, EnemyStateID>
{
public:
    void Enter(Enemy& enemy) override;
    void Update(Enemy& enemy, float elapsedTime) override;
    void Exit(Enemy& enemy) override;

    std::string GetName() const override { return "Berserk"; }
    EnemyStateID GetStateID() const override { return EnemyStateID::BERSERK; }

    StatePriority GetPriority() const override { return StatePriority::OVERRIDE; }

    // 他のすべてのステートを上書きする
    bool CanOverride(const State<Enemy, EnemyStateID>& other) const override {
        return true;
    }

    static BerserkState& Instance() {
        static BerserkState instance;
        return instance;
    }

private:
    BerserkState() = default;

    // バーサクの進行状態
    enum class BerserkPhase {
        JUMP_START,
        INVISIBLE_LOOP,
        BERSERK_LAND,
        BERSERK_IDLE,
        AIM_START,
        AIM_LOOP,
        ATTACK_EXECUTION
    };

    BerserkPhase currentPhase = BerserkPhase::JUMP_START;

    float phaseTimer = 0.0f;

    // バーサク全体の継続時間
    float berserkDuration = 0.0f;

    // バーサク中の基準位置
    DirectX::XMFLOAT3 berserkPosition = { 0.0f, 0.0f, 0.2f };

    // テレポートの開始と終了位置
    DirectX::XMFLOAT3 teleportStartPos = {};
    DirectX::XMFLOAT3 teleportEndPos = {};

    // 攻撃ターゲット位置
    DirectX::XMFLOAT3 meteorTargetPos = {};

    // エフェクトの発動管理
    bool hasTriggeredSlashEffect = false;
    bool hasTriggeredBlastEffect = false;

    float meteorImpactTime = 0.0f;

    // 当たり判定生成の管理
    bool hasSpawnedCollision = false;

    // 待機から攻撃までの時間
    float idleToAttackTime = EnemyConstants::BERSERK_IDLE_TO_ATTACK_TIME;

    // 攻撃の間隔
    float attackCooldown = EnemyConstants::BERSERK_ATTACK_COOLDOWN;

    // サイズ変更の管理
    bool hasScaledUp = false;

    // 危険エリアの判定
    bool playerWasInZoneLastFrame = false;
    bool playerIsInZoneThisFrame = false;

    Effekseer::Handle dangerZoneEffectHandle = -1;

    bool dangerZoneEffectActive = false;
    float dangerZoneEffectTimer = 0.0f;

    // ヒット判定のタイミング
    const float DANGER_HIT_FRAME_TIME =
        EnemyConstants::BERSERK_DANGER_ZONE_FRAMES / EnemyConstants::BERSERK_FRAME_RATE;

    bool hasTriggeredDangerHit = false;

    // 攻撃の進行管理
    int currentAttackSet = CharacterConstants::FIRST_ATTACK;
    int currentAttackPart = EnemyConstants::ATTACK_PART_ONE;

    bool isTransitioning = false;

    // フレーム時間
    const float frameTime = 1.0f / EnemyConstants::BERSERK_FRAME_RATE;

    // 各攻撃の切り替えタイミング
    float slash01To02Time = EnemyConstants::BERSERK_SLASH_FRAME * frameTime;
    float blast01To02Time = EnemyConstants::BERSERK_BLAST_FRAME * frameTime;

    Effekseer::Handle blastEffectHandle = -1;
    bool blastEffectActive = false;

    // 警告エリアと攻撃生成
    DirectX::XMFLOAT3 warningAreaPos = {};

    bool hasSpawnedSlashLightning = false;
    bool hasSpawnedWarningArea = false;
    bool hasSpawnedSlashProjectile = false;

    float projectileDelayTimer = 0.0f;

    // フェーズ更新ヘルパー
    void UpdateJumpStart(Enemy& enemy);
    void UpdateInvisibleLoop(Enemy& enemy);
    void UpdateBerserkLand(Enemy& enemy);
    void UpdateBerserkIdle(Enemy& enemy, float elapsedTime);
    void UpdateAimStart(Enemy& enemy);
    void UpdateAimLoop(Enemy& enemy);
    void UpdateAttackExecution(Enemy& enemy, float elapsedTime);

    // 攻撃実行ヘルパー
    void UpdateSecondAttackSet(Enemy& enemy, const Model::Animation& anim, float elapsedTime);
    void UpdateFirstAttackSet(Enemy& enemy, const Model::Animation& anim);
    void SpawnMeteorImpact(Enemy& enemy);
    void SpawnBlastEffect(Enemy& enemy, const Model::Animation& anim);
    void CheckAttackPartTransition(Enemy& enemy);
    void HandleAttackAnimationEnd(Enemy& enemy);

    // プレイヤーの方向を向く
    void FacePlayer(Enemy& enemy);

    // 攻撃開始
    void StartAttackSequence(Enemy& enemy);

    // 次の攻撃へ移行
    void TransitionToNextAttackPart(Enemy& enemy);

    // エフェクト位置の更新
    void UpdateBlastEffectPosition(Enemy& enemy);

    // 危険エリア内か判定
    bool IsPlayerInDangerZone() const;

    // 危険エリアエフェクトの管理
    void ManageDangerZoneEffect(bool shouldBeActive, float elapsedTime);

    // エフェクト停止
    void StopDangerZoneEffect();
};