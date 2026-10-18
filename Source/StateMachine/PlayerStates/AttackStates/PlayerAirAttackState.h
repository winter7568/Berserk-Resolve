#pragma once
#include "StateMachine/State.h"
#include "GameObjects/Player/Player.h"
#include "AttackData.h"

// 空中攻撃状態
// プレイヤーの空中攻撃コンボを管理する
class PlayerAirAttackState : public State<Player, PlayerStateID>
{
public:
    void Enter(Player& player) override;
    void Update(Player& player, float elapsedTime) override;
    void Exit(Player& player) override;

    std::string GetName() const override { return "AirAttack"; }
    PlayerStateID GetStateID() const override { return PlayerStateID::AIR_ATTACK; }

    StatePriority GetPriority() const override { return StatePriority::ATTACK; }

    bool CanCombineWith(const State<Player, PlayerStateID>& other) const override {
        return other.GetPriority() > GetPriority();
    }

    static PlayerAirAttackState& Instance() {
        static PlayerAirAttackState instance;
        return instance;
    }

    // デバッグ用メソッド
    int GetCurrentAttackIndex() const { return currentAttack; }

private:
    PlayerAirAttackState() = default;

    // 空中攻撃コンボのアニメーション名
    std::vector<PlayerAnim> attackAnimations = {
        PlayerAnim::AIR_ATTACK_01,
        PlayerAnim::AIR_ATTACK_02,
        PlayerAnim::AIR_ATTACK_03,
        PlayerAnim::AIR_ATTACK_04
    };

    // フレームデータ
    const std::vector<AttackData> attackData = {
        {49, 7, 10, 25, 40, true},   // 攻撃1
        {49, 8, 15, 25, 40, true},   // 攻撃2
        {96, 20, 70, 80, 96, true},  // 攻撃3
        {56, 20, 30, -1, -1, false}  // 攻撃4 キャンセル不可
    };

    // 空中攻撃のサウンドマップ
    static constexpr PlayerSound airAttackSoundMap[4] = {
        PlayerSound::AIR_SLASH,    // 攻撃0 1回目の斬撃
        PlayerSound::AIR_SLASH,    // 攻撃1 2回目の斬撃
        PlayerSound::STAFF_SPIN,   // 攻撃2 杖回転
        PlayerSound::AIR_BEAM      // 攻撃3 電撃ビーム
    };

    bool isActive = false;  // 状態がアクティブか

    int currentAttack = 0;           // 現在の攻撃インデックス
    int queuedInputs = 0;            // キューされた入力数
    float inputBufferTime = 0.0f;    // 入力バッファ時間

    bool hasSpawnedProjectileThisAttack = false;  // この攻撃で投射物を生成済みか
    int projectilesSpawnedThisAttack = 0;  // この攻撃で生成した投射物数
    int lastAttackIndex = -1;        // 前回の攻撃インデックス

    bool thirdAttackSoundPlaying = false;  // 3回目攻撃のサウンド再生中か

    // 空中攻撃の投射物データ
    const std::vector<ProjectileAttackData> airProjectileData = {
        // 攻撃1 ホーミング斬撃カプセル
        { attackData[0].hitStart, 1, "air_slash", {}, "staff_tip" },
        // 攻撃2 ホーミング斬撃カプセル
        { attackData[1].hitStart, 1, "air_slash", {}, "staff_tip" },
        // 攻撃3 投射物なし
        { -1, 0, "", {}, "" },
        // 攻撃4 ビームカプセル
        { attackData[3].hitStart, 1, "air_beam", {}, "staff_tip" }
    };

    // 空中投射物データを取得するメソッド
    const ProjectileAttackData& GetAirProjectileData(int attackIndex) {
        if (attackIndex >= 0 && attackIndex < airProjectileData.size()) {
            return airProjectileData[attackIndex];
        }
        static ProjectileAttackData defaultData = { -1, 0, "", {}, "" };
        return defaultData;
    }

    // 杖回転の追跡
    static constexpr int SPIN_BLEND_FRAMES = 2;
    static constexpr float SPIN_BLEND_DURATION = 2.0f;
    bool isSpinning = false;                     // 回転中か
    float totalSpinRotation = DirectX::XMConvertToRadians(1440.0f); // 総回転角度 3回転
    float staffForwardOffset = 0.0f;             // 杖の前方オフセット

    // 抽出したヘルパー関数
    void UpdateTimeScale(int currentAttack);
    void ResetSpawnStateIfNeeded(int currentAttack);
    void SpawnAirProjectile(Player& player, int currentFrame);
    void UpdateWeaponSpin(Player& player, int currentFrame);
    bool CheckCancelWindow(Player& player, int currentFrame);
    void ResetQueuedInputsIfNeeded(int currentFrame);
    void FinishAirAttack(Player& player);
};