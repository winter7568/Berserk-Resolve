#pragma once
#include "GameObjects/Character/Character.h"
#include "GameLib/GamePad.h"
#include "GameLib/Mouse.h"
#include "StateMachine/State.h"
#include "ShaderSystems/TrailSystem.h"
#include "GameSystems/Effect/Effect.h"
#include "GameSystems/Debug/DebugRenderer.h"

class Player : public Character
{
    FRIEND_PLAYER_STATES;

public:
    // シングルトンアクセス
    static Player& Instance() {
        static Player instance;
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
    StateManager<Player, PlayerStateID>& GetStateManager() { return stateManager; }
    const StateManager<Player, PlayerStateID>& GetStateManager() const { return stateManager; }

    // 攻撃状態ID設定
    void SetCurrentAttackStateID(PlayerStateID stateID) {
        currentAttackStateID = stateID;
    }

    // 攻撃状態ID取得
    PlayerStateID GetCurrentAttackStateID() const {
        return currentAttackStateID;
    }
    bool IsAttacking() const;

    // 攻撃・移動状態の一覧
    static const std::vector<PlayerStateID>& GetAttackMovementStateIDs() {
        static std::vector<PlayerStateID> stateIDs = {
            PlayerStateID::IDLE,
            PlayerStateID::RUN,
            PlayerStateID::DASH,
            PlayerStateID::FALLING,
            PlayerStateID::JUMP,
            PlayerStateID::AIR_ATTACK,
            PlayerStateID::PROJECTILE_ATTACK,
            PlayerStateID::ATTACK
        };
        return stateIDs;
    }

    // 攻撃・移動状態をまとめて消す
    void ClearAttackAndMovementStates() {
        ClearStatesByIDs(GetAttackMovementStateIDs(), stateManager);
    }

    // アニメーション番号取得
    int GetAnimIndex(PlayerAnim anim) const;

    // 入力処理
    bool InputLockOn();          // ロックオン
    bool InputJump();            // ジャンプ
    bool InputMove(DirectX::XMVECTOR& outWorldDir); // 移動
    bool InputDash();            // ダッシュ
    bool InputAttack();          // 攻撃
    bool InputMeleeAttack();     // 近接攻撃
    bool InputProjectileAttack(); // 遠距離攻撃

    WeaponObject& GetWeapon() override { return staff; }
    const WeaponObject& GetWeapon() const override { return staff; }
    void UpdateWeaponAttachment() override;  // オーバーライド（スピン処理付き）

    // 武器を持っているか
    const char* GetLeftHandBoneName() const override { return "index_02_l"; }
    bool HasWeapon() const override { return true; }

    // 武器の行列取得
    DirectX::XMFLOAT4X4 GetWeaponTransform() const override { return staff.transform; }

    // コリジョン取得
    DebugCapsuleParams& GetWeaponCapsuleParams() override { return weaponCapsule; }
    DebugSphereParams& GetWeaponSphereParams() override { return weaponSphere; }

    void SetWeponCapsuleParams(const DebugCapsuleParams& params) { weaponCapsule = params; }

    bool GetWeaponCapsule(DirectX::XMFLOAT3& outBase, DirectX::XMFLOAT3& outTip, float& outRadius);
    bool GetWeaponSphere(DirectX::XMFLOAT3& outCenter, float& outRadius);

    // ヒット判定ON/OFF
    void EnableHitbox(int attackIndex) { SetWeaponHitActive(true); }
    void DisableHitbox(int attackIndex) { SetWeaponHitActive(false); }

    // 武器モデル取得
    Model* GetWeaponModel() const { return staff.model.get(); }

    // 武器の位置
    DirectX::XMFLOAT3 GetWeaponBasePosition() const;
    DirectX::XMFLOAT3 GetWeaponTipPosition() const;

    // 移動関係
    int GetJumpCount() const { return jumpCount; }
    void SetJumpCount(int count) { jumpCount = count; }
    float GetDashScale(float elapsedTime);
    void UpdateLockOnByDistance();
    void AutoFaceEnemy(float elapsedTime, float maxDistance);
    void TransitionToMovementState(bool isMoving);

    // カウンター可能か
    bool CanEnterCounterState();

    // 武器スピン
    void StartWeaponSpin(float speed);
    void StopWeaponSpin();
    void UpdateWeaponSpin(float elapsedTime);
    void SetStaffForwardOffset(float offset) { staffForwardOffset = offset; }
    void RenderWeaponSpinDebugUI();

    // マナ関連
    float GetCurrentMana() const { return currentMana; }
    float GetMaxMana() const { return maxMana; }
    bool TryConsumeMana(float cost);
    void UpdateMana(float elapsedTime);
    void ResetMana() { currentMana = maxMana; }
    void RenderManaDebug();

    // トレイル描画
    void RenderSwordTrail(const RenderContext& rc);
    void RenderSwordTrailDebugUI();

    // 音再生
    void PlayPlayerSound(PlayerSound sound, float volume, bool loop = false);
    void StopPlayerSound(PlayerSound sound);
    void CleanupAudio();

    // スポーン更新
    bool UpdateSpawn(float elapsedTime);

    // デバッグ設定
    void SetDebugHitRegistration(bool enable) { debugHitRegistration = enable; }
    bool IsDebugHitRegistration() const { return debugHitRegistration; }

    void SetDebugWeaponHitActive(bool active) { debugWeaponHitActive = active; }
    bool IsDebugWeaponHitActive() const { return debugWeaponHitActive; }

private:
    Player() = default;
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    // 保護されたメソッド - オーバーライド
    // ルートモーション
    void RootMotion(float elapsedTime) override;

    // プライベートメソッド - セットアップヘルパー
    void LoadModels(ID3D11Device* device);
    void SetupDissolveEffect(ID3D11Device* device);
    void SetupInitialTransform();
    void SetupWeapon();
    void SetupColliders();
    void SetupPlayerStats();
    void SetupStateManager();
    void SetupTrailSystem(ID3D11Device* device);
    void LoadAudio();

    // プライベートメソッド - リセットヘルパー
    void ResetTransform();
    void ResetStats();
    void ResetState();
    void ResetAttackState();
    void ResetMovementFlags();
    void ResetWeaponState();
    void ResetSpawnState();

    // プライベートメソッド - 更新ヘルパー
    void UpdateStaffParticlesIfAttacking(float elapsedTime);
    void CalculateRootMotion(const Model::Animation& animation, float elapsedTime);
    void ApplyDashMotionScale(DirectX::XMFLOAT3& motionDelta, float elapsedTime);
    void ApplyRunMotionScale(DirectX::XMFLOAT3& motionDelta);
    void UpdateStaffParticles(float elapsedTime)const;

    // プライベートメソッド - レンダーデバッグヘルパー
    void RenderWeaponCapsuleDebug(const RenderContext& rc, DebugRenderer& debugRenderer);
    void RenderWeaponSphereDebug(const RenderContext& rc, DebugRenderer& debugRenderer);

    // メンバ変数 - 状態管理
    StateManager<Player, PlayerStateID> stateManager{ *this };
    PlayerStateID currentAttackStateID = PlayerStateID::NONE;

    // メンバ変数 - 武器（スタッフ）
    Character::WeaponObject staff;

    // 武器コリジョン
    DebugCapsuleParams weaponCapsule;
    DebugSphereParams weaponSphere;

    // ヒット時の色
    DirectX::XMFLOAT4 weaponHitColor = { 0.0f, 1.0f, 0.0f, 1.0f };
    DirectX::XMFLOAT4 weaponNormalColor = { 1.0f, 0.0f, 0.0f, 1.0f };

    // メンバ変数 - 移動パラメータ
    int jumpCount = 0;
    float jumpPower = PlayerConstants::JUMP_POWER;
    float doubleJumpPower = PlayerConstants::DOUBLE_JUMP_POWER;
    float airSpeed = PlayerConstants::AIR_SPEED;
    float airControl = PlayerConstants::AIR_CONTROL;
    float dashHoldTime = 0.0f;

    // 空中移動フラグ
    bool hasAirDashed = false;
    bool jumpSkipStart = false;

    // メンバ変数 - 武器位置調整
    DirectX::XMFLOAT3 capsuleOffset = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 capsuleAngle = { 0.0f, 0.0f, 0.0f };
    float weaponTipOffset = -0.8f;

    // メンバ変数 - デバッグ
    float debugSpinAngleX = -DirectX::XM_PIDIV4;
    float debugSpinAngleY = 0.0f;
    float debugSpinAngleZ = DirectX::XM_PIDIV2;
    bool showWeaponSpinDebug = true;
    bool debugHitRegistration = false;
    bool debugWeaponHitActive = false;
    bool isLockOnActive = false;

    // メンバ変数 - スタッフエフェクト
    DirectX::XMFLOAT3 lastStaffTipPos = { 0, 0, 0 };
    float staffParticleTimer = 0.0f;

    // メンバ変数 - 武器スピン状態
    bool isWeaponSpinning = false;
    float weaponSpinAngle = 0.0f;
    float weaponSpinSpeed = 0.0f;
    float weaponTotalSpinAngle = 0.0f;
    float staffForwardOffset = 0.0f;

    // メンバ変数 - トレイル
    TrailSystem staffTrail;
    DirectX::XMFLOAT3 staffTrailTipOffset = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 staffTrailBaseOffset = { 0.0f, 0.0f, 0.0f };

    // メンバ変数 - マナ
    float currentMana = PlayerConstants::MAX_MANA;
    float maxMana = PlayerConstants::MAX_MANA;
    float manaRegenRate = PlayerConstants::MANA_REGEN_RATE;

    // メンバ変数 - スポーン
    Effekseer::Handle spawnEffectHandle = -1;
    bool spawnEffectPlayed = false;
    float spawnTimer = 0.0f;
    bool spawnCompleted = false;

    // メンバ変数 - 音データ
    std::map<PlayerSound, AudioSource*> sounds;
};