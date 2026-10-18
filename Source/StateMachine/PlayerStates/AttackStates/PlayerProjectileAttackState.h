#pragma once
#include "StateMachine/State.h"
#include "AttackData.h"
#include "GameObjects/Player/Player.h"
#include "GameSystems/Projectile/ProjectileManager.h"

// 遠距離攻撃状態
// プレイヤーの投射物攻撃コンボを管理する
class PlayerProjectileAttackState : public State<Player, PlayerStateID>
{
public:
    void Enter(Player& player) override;
    void Update(Player& player, float elapsedTime) override;
    void Exit(Player& player) override;

    std::string GetName() const override { return "ProjectileAttack"; }
    PlayerStateID GetStateID() const override { return PlayerStateID::PROJECTILE_ATTACK; }

    StatePriority GetPriority() const override { return StatePriority::ATTACK; }

    bool CanCombineWith(const State<Player, PlayerStateID>& other) const override {
        return other.GetPriority() > GetPriority();
    }

    static PlayerProjectileAttackState& Instance() {
        static PlayerProjectileAttackState instance;
        return instance;
    }

    // デバッグ用メソッド
    int GetCurrentAttackIndex() const { return currentAttack; }

private:
    PlayerProjectileAttackState() = default;

    // アニメーション名の配列
    std::vector<PlayerAnim> attackAnimations = {
        PlayerAnim::PROJECTILE_ATTACK_01,
        PlayerAnim::PROJECTILE_ATTACK_02,
        PlayerAnim::PROJECTILE_ATTACK_03,
        PlayerAnim::PROJECTILE_ATTACK_04
    };

    // フレームデータ 60fps想定
    const std::vector<AttackData> attackData = {
        {124, 10, 20, 35, 53, true},   // 攻撃1
        {124, 20, 40, 34, 46, true},   // 攻撃2
        {150,  5, 15, 65, 77, true},   // 攻撃3
        {154,  0, 50, -1, -1, false}   // 攻撃4 キャンセル不可
    };

    // 攻撃ごとのサウンドマップ
    static constexpr PlayerSound attackSoundMap[4] = {
        PlayerSound::PISTOL,      // 攻撃0
        PlayerSound::STAFF,       // 攻撃1
        PlayerSound::BOW_QUICK,   // 攻撃2
        PlayerSound::BOW_LAUNCH   // 攻撃3
    };

    int currentAttack = 0;           // 現在の攻撃インデックス
    int queuedInputs = 0;            // キューされた入力数
    float inputBufferTime = 0.0f;    // 入力バッファ時間

    bool hasSpawnedProjectileThisAttack = false;  // この攻撃で投射物を生成済みか
    int lastAttackIndex = -1;        // 前回の攻撃インデックス
    int projectilesSpawnedThisAttack = 0;  // この攻撃で生成した投射物数

    // 全攻撃の投射物データ
    const std::vector<ProjectileAttackData> projectileData = {
        // 攻撃1 指からピストル
        { 20, 1, "pistol", {}, "hand_l" },
        // 攻撃2 杖の先端から投射物
        { 25, 1, "staff", {}, "staff_tip" },
        // 攻撃3 3連続の素早い弓矢
        { 0, 3, "bow_quick", { 40.0f, 50.0f, 60.0f }, "staff_tip" },
        // 攻撃4 チャージされた弓矢
        { 50, 1, "bow_charged", {}, "staff_base" }
    };

    // 投射物データを取得するメソッド
    const ProjectileAttackData& GetProjectileData(int attackIndex) {
        if (attackIndex >= 0 && attackIndex < projectileData.size()) {
            return projectileData[attackIndex];
        }
        static ProjectileAttackData defaultData = { 20, 1, "staff", {}, "staff_tip" };
        return defaultData;
    }

    // 抽出したヘルパー関数
    bool CheckInputOverrides(Player& player);
    void UpdateTimeScale(int currentAttack);
    void ResetSpawnStateIfNeeded(int currentAttack);
    void SpawnProjectile(Player& player, int currentFrame);
    bool CheckCancelWindow(Player& player, int currentFrame);
    void FinishAttack(Player& player);
};