#include "ProjectileManager.h"
#include "Projectile.h"
#include "GameLib/Graphics.h"
#include "GameSystems/Collision/CollisionManager.h"
#include "GameSystems/Effect/EffectManager.h"

void ProjectileManager::Initialize(ID3D11Device* device)
{
    // モデル事前読み込み
    projectileModels["player_bullet"] = std::make_shared<Model>(
        device,
        ProjectileManagerConstants::PLAYER_BULLET_PATH,
        ProjectileManagerConstants::PLAYER_BULLET_SCALE
    );
    projectileModels["player_bullet"]->PlayAnimation(0, true); // ループ再生

    projectileModels["enemy_orb"] = std::make_shared<Model>(
        device,
        ProjectileManagerConstants::ENEMY_ORB_PATH,
        ProjectileManagerConstants::ENEMY_ORB_SCALE
    );

    projectileModels["air_slash"] = std::make_shared<Model>(
        device,
        ProjectileManagerConstants::AIR_SLASH_PATH,
        ProjectileManagerConstants::AIR_SLASH_SCALE
    );

    projectileModels["bow_charged"] = std::make_shared<Model>(
        device,
        ProjectileManagerConstants::BOW_CHARGED_PATH,
        ProjectileManagerConstants::BOW_CHARGED_SCALE
    );
}

void ProjectileManager::Update(float elapsedTime)
{
    // 弾更新
    for (size_t i = 0; i < projectiles.size(); ++i)
    {
        auto& projectile = projectiles[i];
        if (projectile->IsActive())
        {
            projectile->Update(elapsedTime);
        }
    }

    // 衝突判定
    CheckProjectileCollisions();

    // 非アクティブ削除
    auto it = projectiles.begin();
    while (it != projectiles.end())
    {
        if (!(*it)->IsActive())
        {
            it = projectiles.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // 上限チェック
    if (projectiles.size() > ProjectileManagerConstants::MAX_PROJECTILES)
    {
        projectiles.erase(
            projectiles.begin(),
            projectiles.begin() + (projectiles.size() - ProjectileManagerConstants::MAX_PROJECTILES)
        );
    }
}

void ProjectileManager::Render(const RenderContext& rc)
{
    // 描画
    for (auto& projectile : projectiles)
    {
        if (projectile->IsActive())
        {
            projectile->Render(rc);
        }
    }
}

void ProjectileManager::CheckProjectileCollisions()
{
    auto& collisionManager = CollisionManager::Instance();

    for (auto& projectile : projectiles)
    {
        if (!projectile->IsActive()) continue;

        // 発射元で判定分岐
        if (projectile->GetOwnerType() == "player")
        {
            collisionManager.CheckProjectileEnemyCollision(*projectile, Enemy::Instance()); // 敵に当たる
        }
        else if (projectile->GetOwnerType() == "enemy")
        {
            collisionManager.CheckProjectilePlayerCollision(*projectile, Player::Instance()); // プレイヤーに当たる
        }
    }
}

void ProjectileManager::CreateProjectile(
    const std::string& ownerType,
    const std::string& projectileType,
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& direction)
{
    auto projectile = std::make_shared<Projectile>(ownerType, projectileType, position, direction);
    projectiles.push_back(projectile);

    // プレイヤー発射フラグ
    if (ownerType == "player")
    {
        playerJustFiredProjectile = true;
    }
}

void ProjectileManager::CreateCapsuleProjectile(
    const std::string& ownerType,
    const std::string& projectileType,
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& direction,
    const DirectX::XMFLOAT3& orientation)
{
    auto projectile = std::make_shared<Projectile>(ownerType, projectileType, position, direction);
    projectile->SetOrientation(orientation); // 向き設定
    projectiles.push_back(projectile);

    // プレイヤー発射フラグ
    if (ownerType == "player")
    {
        playerJustFiredProjectile = true;
    }
}

void ProjectileManager::ClearAllProjectiles()
{
    for (auto& projectile : projectiles)
    {
        Effekseer::Handle handle = projectile->GetVisualEffectHandle();
        if (handle >= 0)
        {
            EffectManager::Instance().StopEffect(handle); // エフェクト停止
        }
    }

    projectiles.clear();
    playerJustFiredProjectile = false;
}

std::shared_ptr<Model> ProjectileManager::GetProjectileModel(const std::string& modelType)
{
    auto it = projectileModels.find(modelType);
    if (it != projectileModels.end())
    {
        return it->second;
    }
    return nullptr;
}