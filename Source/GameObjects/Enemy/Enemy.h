#pragma once
#include <deque>
#include <map> 

#include "GameObjects/Character/Character.h"
#include "GameLib/GamePad.h"
#include "GameLib/Mouse.h"
#include "ShaderSystems/TrailSystem.h"
#include "StateMachine/EnemyStates/MoveStates.h"
#include "GameSystems/Debug/DebugRenderer.h"

class Enemy : public Character
{
    FRIEND_ENEMY_STATES;

public:
    // シングルトンアクセス
    static Enemy& Instance() {
        static Enemy instance;
        return instance;
    }

    // メインフロー - オーバーライド
    void Initialize() override;
    void Update(float elapsedTime) override;
    void Reset() override;
    void Render(const RenderContext& rc) override;

    // 現在の状態名を返す
    std::string GetCurrentStateName() const override {
        return stateManager.GetCurrentStateName();
    }

    // ダメージ処理
    void ApplyDamage(int damage, float invincibleTime, const HitData& hitData);

    // デバッグ描画
    void RenderDebugPrimitive(const RenderContext& rc, ShapeRenderer* renderer);

    // 状態管理取得
    StateManager<Enemy, EnemyStateID>& GetStateManager() { return stateManager; }
    const StateManager<Enemy, EnemyStateID>& GetStateManager() const { return stateManager; }

    // 攻撃状態ID設定
    void SetCurrentAttackStateID(EnemyStateID stateID) {
        currentAttackStateID = stateID;
    }

    // 攻撃状態ID取得
    EnemyStateID GetCurrentAttackStateID() const {
        return currentAttackStateID;
    }

    // 攻撃・移動状態の一覧
    static const std::vector<EnemyStateID>& GetAttackMovementStateIDs() {
        static std::vector<EnemyStateID> stateIDs = {
            EnemyStateID::IDLE,
            EnemyStateID::WALK,
            EnemyStateID::DASH,
            EnemyStateID::ATTACK,
            EnemyStateID::POWER_CHARGE,
            EnemyStateID::STARFE_ATTACK,
            EnemyStateID::COMBO_ATTACK,
            EnemyStateID::TELEPORT,
            EnemyStateID::REGULAR_HIT,
            EnemyStateID::LAUNCH_HIT,
            EnemyStateID::STUN
        };
        return stateIDs;
    }

    // 攻撃・移動状態をまとめて消す
    void ClearAttackAndMovementStates() {
        ClearStatesByIDs(GetAttackMovementStateIDs(), stateManager);
    }

    // アニメーション番号取得
    int GetAnimIndex(EnemyAnim anim) const;

    WeaponObject& GetWeapon() override { return sword; }
    const WeaponObject& GetWeapon() const override { return sword; }

    // 武器を持っているか
    const char* GetLeftHandBoneName() const override { return "middle_02_l"; }
    bool HasWeapon() const override { return true; }

    // 武器の行列取得
    DirectX::XMFLOAT4X4 GetWeaponTransform() const override { return sword.transform; }

    // コリジョン取得
    DebugCapsuleParams& GetWeaponCapsuleParams() override { return weaponCapsule; }
    DebugSphereParams& GetWeaponSphereParams() override { return weaponSphere; }

    bool GetWeaponCapsule(DirectX::XMFLOAT3& outBase, DirectX::XMFLOAT3& outTip, float& outRadius);

    // 武器モデル取得
    Model* GetWeaponModel() const { return sword.model.get(); }

    // カウンターウィンドウ
    bool IsInCounterWindow() const { return isInCounterWindow; }
    void SetInCounterWindow(bool inWindow) { isInCounterWindow = inWindow; }

    // スラッシュインデックス
    int GetCurrentSlashIndex() const { return currentSlashIndex; }
    void SetCurrentSlashIndex(int index) { currentSlashIndex = index; }

    // プレイヤー方向を計算する
    bool IsFacingPlayer() const;
    float CalculateDistanceToPlayer() const;
    DirectX::XMFLOAT2 GetDirectionToPlayer() const;
    // プレイヤーの方向を向く（補間あり）
    void SmoothlyTrackPlayer(float elapsedTime, float turnSpeed = EnemyConstants::TURN_SPEED_NORMAL);

    // AI判断
    bool ShouldBlock(float distanceToPlayer, bool isPlayerAttacking) const;
    bool ShouldDash(float distanceToPlayer) const;
    bool ShouldCounterAfterBlock() const;
    bool ShouldUseSpecialAttack() const;
    bool ShouldTeleport() const;

    // 性格パラメータ取得
    float GetPersonalityBlockChance() const;
    float GetPersonalityDashChance() const;
    float GetPersonalityCounterChance() const;
    float GetPersonalitySpecialAttackChance() const;
    float GetPersonalityTeleportChance() const;

    // 発光色制御
    void SetEmissiveColor(const DirectX::XMFLOAT3& color);
    void ResetEmissiveColor();
    void FlashEmissive(const DirectX::XMFLOAT3& color, float duration);

    // 剣のポーズ
    void ApplyBerserkSwordPose();
    void ApplyDefaultSwordPose();

    // トレイル描画
    void RenderSwordTrail(const RenderContext& rc);
    void RenderSwordTrailDebugUI();
    void RenderSwordPositioningDebugUI();
    void RenderPositioningBrainDebugUI();

    // 音再生
    void PlayEnemySound(EnemySound sound, float volume, bool loop = false);
    void StopEnemySound(EnemySound sound);
    void CleanupAudio();

    // シネマティック出現
    void StartCinematicSpawn();
    void UpdateCinematicSpawn(float elapsedTime);
    bool IsCinematicSpawnComplete() const { return cinematicSpawnComplete; }

    // パワーチャージエフェクト
    void StartPowerChargeEffect(DirectX::XMFLOAT3& startPosition);
    void StopPowerChargeEffect();

    // 状態フラグ
    bool IsBerserk() const { return health < (EnemyConstants::MAX_HEALTH / 2); }
    bool HasBeenKnockedDown() const { return hasBeenKnockedDown; }
    void SetKnockedDown(bool knocked) { hasBeenKnockedDown = knocked; }
    bool IsbodyGlow() const { return bodyGlow; }
    void SetbodyGlow(bool glow) { bodyGlow = glow; }

    // 武器オブジェクトアクセサ
    const WeaponObject& GetSword() const { return sword; }
    WeaponObject& GetSword() { return sword; }

    // デバッグ情報構造体
    struct DebugInfo {
        std::string currentDecision = "None";
        std::string lastAction = "None";
        float lastDecisionTime = 0.0f;
        std::vector<std::string> decisionHistory;
    };

    // デバッグ情報アクセサ（デバッグUI用）
    const DebugInfo& GetPositioningDebug() const { return positioningDebug; }
    DebugInfo& GetPositioningDebug() { return positioningDebug; }

private:
    Enemy() = default;
    Enemy(const Enemy&) = delete;
    Enemy& operator=(const Enemy&) = delete;

    // 保護されたメソッド - オーバーライド
    void RootMotion(float elapsedTime) override;

    // プライベートメソッド - セットアップヘルパー
    void LoadModels(ID3D11Device* device);
    void SetupDissolveEffect(ID3D11Device* device);
    void SetupInitialTransform();
    void SetupColliders();
    void SetupStateManager();
    void SetupStats();
    void SetupTrailSystem(ID3D11Device* device);
    void LoadAudio();

    // プライベートメソッド - リセットヘルパー
    void ResetTransform();
    void ResetStats();
    void ResetState();
    void ResetAttackState();
    void ResetCombatFlags();
    void ResetSpawnAndDownState();

    // プライベートメソッド - 更新ヘルパー
    void UpdateEmissiveFlash(float elapsedTime);
    void CalculateRootMotion(const Model::Animation& animation);
    void ApplyMovementScales(DirectX::XMFLOAT3& motionDelta);

    // プライベートメソッド - AIヘルパー
    EnemyPositioningState& GetPositioningBrain() const;

    // プライベートメソッド - レンダーデバッグヘルパー
    void RenderWeaponCapsuleDebug(const RenderContext& rc, DebugRenderer& debugRenderer);

    // メンバ変数 - 状態管理
    StateManager<Enemy, EnemyStateID> stateManager{ *this };
    EnemyStateID currentAttackStateID = EnemyStateID::NONE;

    // メンバ変数 - 武器（剣）
    WeaponObject sword;

    // 武器コリジョン
    DebugCapsuleParams weaponCapsule;
    DebugSphereParams weaponSphere;

    // メンバ変数 - 戦闘状態
    bool isInCounterWindow = false;
    int currentSlashIndex = 0;

    // メンバ変数 - トレイル
    TrailSystem swordTrail;
    DirectX::XMFLOAT3 swordTrailTipOffset = EnemyConstants::TRAIL_TIP_OFFSET;
    DirectX::XMFLOAT3 swordTrailBaseOffset = EnemyConstants::TRAIL_BASE_OFFSET;

    // メンバ変数 - 発光色
    DirectX::XMFLOAT3 defaultEmissiveColor = EnemyConstants::DEFAULT_EMISSIVE_COLOR;
    DirectX::XMFLOAT3 currentEmissiveColor = EnemyConstants::DEFAULT_EMISSIVE_COLOR;
    float emissiveFlashTimer = 0.0f;
    DirectX::XMFLOAT3 flashColor = EnemyConstants::FLASH_COLOR;

    // メンバ変数 - シネマティック出現
    float cinematicSpawnTimer = 0.0f;
    bool cinematicSpawnActive = false;
    bool cinematicSpawnComplete = false;

    // メンバ変数 - 状態フラグ
    bool hasBeenKnockedDown = false;
    bool bodyGlow = false;

    // メンバ変数 - 音データ
    std::map<EnemySound, AudioSource*> enemySounds;

    // メンバ変数 - デバッグ情報
    DebugInfo positioningDebug;
};