#pragma once
#include <DirectXMath.h>

#include "GameObjects/Player/Player.h"
#include "GameObjects/Enemy/Enemy.h"
#include "Collision.h"
#include "GameSystems/Projectile/Projectile.h"

class CollisionManager
{
public:
    static CollisionManager& Instance()
    {
        static CollisionManager instance;
        return instance;
    }

    CollisionManager(const CollisionManager&) = delete;
    CollisionManager& operator=(const CollisionManager&) = delete;

    // ゲームオブジェクト間のメイン衝突判定
    bool CheckPlayerEnemyCollision(Player& player, Enemy& enemy);        // 本体同士の衝突判定
    bool CheckWeaponEnemyCollision(Player& player, Enemy& enemy);        // プレイヤー武器と敵の衝突判定
    bool CheckProjectileEnemyCollision(Projectile& projectile, Enemy& enemy); // 弾と敵の衝突判定
    bool CheckEnemyWeaponPlayerCollision(Enemy& enemy, Player& player);  // 敵武器とプレイヤーの衝突判定
    bool CheckProjectilePlayerCollision(Projectile& projectile, Player& player); // 弾とプレイヤーの衝突判定

    // 衝突状態の取得
    bool IsPlayerEnemyColliding() const { return playerEnemyColliding; }
    bool IsWeaponEnemyColliding() const { return weaponEnemyColliding; }
    bool IsProjectileEnemyColliding() const { return projectileEnemyColliding; }

    // 弾がプレイヤーに当たったときの処理
    void OnProjectileHitPlayer(Projectile& projectile, Player& player);

    // 毎フレームの衝突フラグをリセット
    void ResetFrameState()
    {
        playerEnemyColliding = false;
        weaponEnemyColliding = false;
        wasWeaponEnemyColliding = false;
    }

private:
    CollisionManager() = default;
    ~CollisionManager() = default;

    // 衝突判定定数
    struct CollisionConstants
    {
        static constexpr float MIN_DISTANCE = 0.001f;
        static constexpr float MIN_WEIGHT = 1.0f;
    };

    // 衝突状態フラグ（現在フレーム）
    bool playerEnemyColliding = false;
    bool weaponEnemyColliding = false;
    bool wasWeaponEnemyColliding = false;  // 前フレームの状態
    bool enemyWeaponPlayerColliding = false;
    bool wasEnemyWeaponPlayerColliding = false;
    bool projectileEnemyColliding = false;
    bool wasProjectileEnemyColliding = false;
    bool projectilePlayerColliding = false;
    bool wasProjectilePlayerColliding = false;

    // 内部処理用ヘルパー関数
    void OnProjectileHitEnemy(Projectile& projectile, Enemy& enemy);
    bool CheckCapsuleProjectileVsEnemy(Projectile& projectile, Enemy& enemy);
    bool CheckBeamProjectileVsEnemy(Projectile& projectile, Enemy& enemy);
    void GetProjectileCapsuleEndpoints(Projectile& projectile, DirectX::XMFLOAT3& outBase, DirectX::XMFLOAT3& outTip);
};