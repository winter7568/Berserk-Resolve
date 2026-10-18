#pragma once
#include <DirectXMath.h>
#include <vector>
#include <memory>

#include "GameLib/RenderContext.h"
#include "GameObjects/Enemy/Enemy.h"

class Projectile;

class ProjectileManager
{
public:
    static ProjectileManager& Instance()
    {
        static ProjectileManager instance;
        return instance;
    }

    // コピー禁止
    ProjectileManager(const ProjectileManager&) = delete;
    ProjectileManager& operator=(const ProjectileManager&) = delete;

    void Initialize(ID3D11Device* device); // 初期化
    void Update(float elapsedTime);        // 更新
    void Render(const RenderContext& rc);  // 描画

    void CheckProjectileCollisions(); // 衝突判定

    // 通常弾生成
    void CreateProjectile(
        const std::string& ownerType,      // 発射元
        const std::string& projectileType, // 弾種類
        const DirectX::XMFLOAT3& position, // 位置
        const DirectX::XMFLOAT3& direction // 方向
    );

    // カプセル弾生成
    void CreateCapsuleProjectile(
        const std::string& ownerType,
        const std::string& projectileType,
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& direction,
        const DirectX::XMFLOAT3& orientation = { 0.0f, 0.0f, 0.0f } // 向き
    );

    // 弾追加
    void AddProjectile(std::shared_ptr<Projectile> projectile) {
        projectiles.push_back(projectile);
    }

    void ClearAllProjectiles(); // 全削除

    // モデル取得
    std::shared_ptr<Model> GetProjectileModel(const std::string& modelType);

    const std::vector<std::shared_ptr<Projectile>>& GetProjectiles() const { return projectiles; }

    // プレイヤー発射フラグ
    void SetPlayerJustFiredProjectile(bool fired) { playerJustFiredProjectile = fired; }
    bool GetPlayerJustFiredProjectile() const { return playerJustFiredProjectile; }
    void ResetPlayerJustFiredProjectile() { playerJustFiredProjectile = false; }

private:
    ProjectileManager() = default;
    ~ProjectileManager() = default;

    struct ProjectileManagerConstants
    {
        // モデルパス
        static constexpr const char* PLAYER_BULLET_PATH = "Data/Model/Sphere/Bullet.gltf";
        static constexpr const char* ENEMY_ORB_PATH = "Data/Model/abyssal_sword/scene.gltf";
        static constexpr const char* AIR_SLASH_PATH = "Data/Model/Slash/RedSlash.gltf";
        static constexpr const char* BOW_CHARGED_PATH = "Data/Model/ProjectileSword/scene.gltf";

        // モデルスケール
        static constexpr float PLAYER_BULLET_SCALE = 2.0f;
        static constexpr float ENEMY_ORB_SCALE = 0.008f;
        static constexpr float AIR_SLASH_SCALE = 1.0f;
        static constexpr float BOW_CHARGED_SCALE = 1.0f;

        // 管理設定
        static constexpr size_t MAX_PROJECTILES = 100;
    };
    std::vector<std::shared_ptr<Projectile>> projectiles; // 弾リスト

    std::unordered_map<std::string, std::shared_ptr<Model>> projectileModels; // モデル管理
    bool playerJustFiredProjectile = false; // 発射直後フラグ
};