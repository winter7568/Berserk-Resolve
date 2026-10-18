#include "CollisionManager.h"
#include "GameSystems/HitManagement/HitManager.h"
#include "StateMachine/PlayerStates/PlayerHitState.h"
#include "GameSystems/Particle/ParticleManager.h"

bool CollisionManager::CheckPlayerEnemyCollision(Player & player, Enemy & enemy)
{
    // プレイヤーのカプセル取得
    DirectX::XMFLOAT3 playerBase, playerTip;
    player.GetBodyCapsuleEndpoints(playerBase, playerTip);
    float playerRadius = player.GetScaledBodyRadius();
    // エネミーのカプセル取得
    DirectX::XMFLOAT3 enemyBase, enemyTip;
    enemy.GetBodyCapsuleEndpoints(enemyBase, enemyTip);
    float enemyRadius = enemy.GetScaledBodyRadius();
    DirectX::XMFLOAT3 outEnemyPosition;
    bool result = Collision::IntersectCapsuleVsCapsule(
        playerBase, playerTip, playerRadius,
        enemyBase, enemyTip, enemyRadius,
        outEnemyPosition
    );
    playerEnemyColliding = result;
    if (result)
    {
        DirectX::XMFLOAT3 playerPos = player.GetPosition();
        DirectX::XMFLOAT3 enemyPos = enemy.GetPosition();
        float playerWeight = (player.GetWeight() > 0.0f) ? player.GetWeight() : CollisionConstants::MIN_WEIGHT;
        float enemyWeight = (enemy.GetWeight() > 0.0f) ? enemy.GetWeight() : CollisionConstants::MIN_WEIGHT;
        // 水平方向のみの押し出し
        DirectX::XMFLOAT3 pushDir = {
            enemyPos.x - playerPos.x,
            0.0f,
            enemyPos.z - playerPos.z
        };
        float distance = sqrt(pushDir.x * pushDir.x + pushDir.z * pushDir.z);
        if (distance < CollisionConstants::MIN_DISTANCE) distance = CollisionConstants::MIN_DISTANCE;
        // 正規化
        pushDir.x /= distance;
        pushDir.z /= distance;
        // めり込み量
        float sumRadii = player.GetScaledBodyRadius() + enemy.GetScaledBodyRadius();
        float penetration = sumRadii - distance;
        if (penetration > 0.0f)
        {
            float totalWeight = playerWeight + enemyWeight;
            float playerPushRatio = enemyWeight / totalWeight;
            float enemyPushRatio = playerWeight / totalWeight;
            // プレイヤーとエネミーの位置補正
            playerPos.x -= pushDir.x * penetration * playerPushRatio;
            playerPos.z -= pushDir.z * penetration * playerPushRatio;
            enemyPos.x += pushDir.x * penetration * enemyPushRatio;
            enemyPos.z += pushDir.z * penetration * enemyPushRatio;
            player.SetPosition(playerPos);
            enemy.SetPosition(enemyPos);
        }
    }
    return result;
}

bool CollisionManager::CheckWeaponEnemyCollision(Player& player, Enemy& enemy)
{
    bool shouldCheckCollision = player.IsWeaponHitActive();
    if (!shouldCheckCollision)
    {
        weaponEnemyColliding = false;
        return false;
    }
    // 武器の球情報取得
    DirectX::XMFLOAT3 sphereCenter;
    float sphereRadius;
    if (!player.GetWeaponSphere(sphereCenter, sphereRadius))
    {
        weaponEnemyColliding = false;
        return false;
    }
    // エネミーのカプセル取得
    DirectX::XMFLOAT3 enemyBase, enemyTip;
    enemy.GetBodyCapsuleEndpoints(enemyBase, enemyTip);
    float enemyRadius = enemy.GetScaledBodyRadius();
    DirectX::XMFLOAT3 outEnemyPosition;
    // 球とカプセルの判定
    weaponEnemyColliding = Collision::IntersectSphereVsCapsule(
        sphereCenter, sphereRadius,
        enemyBase, enemyTip, enemyRadius,
        outEnemyPosition
    );
    // デバッグ用カラー変更
    if (weaponEnemyColliding)
    {
        auto weaponParams = player.GetWeaponCapsuleParams();
        weaponParams.color = { 1.0f, 1.0f, 0.0f, 1.0f };
    }
    else
    {
        auto weaponParams = player.GetWeaponCapsuleParams();
        weaponParams.color = { 1.0f, 0.0f, 0.0f, 1.0f };
    }
    wasWeaponEnemyColliding = weaponEnemyColliding;
    return weaponEnemyColliding;
}

bool CollisionManager::CheckProjectileEnemyCollision(Projectile& projectile, Enemy& enemy)
{
    // 弾の形状取得
    ProjectileShape shape = projectile.GetShape();
    // エネミーのカプセル取得
    DirectX::XMFLOAT3 enemyBase, enemyTip;
    enemy.GetBodyCapsuleEndpoints(enemyBase, enemyTip);
    float enemyRadius = enemy.GetScaledBodyRadius();
    bool collision = false;
    DirectX::XMFLOAT3 outEnemyPosition;
    // 弾の種類ごとに判定を分ける
    if (shape == ProjectileShape::SPHERE)
    {
        collision = Collision::IntersectSphereVsCapsule(
            projectile.GetPosition(), projectile.GetRadius(),
            enemyBase, enemyTip, enemyRadius,
            outEnemyPosition
        );
    }
    else if (shape == ProjectileShape::CAPSULE)
    {
        collision = CheckCapsuleProjectileVsEnemy(projectile, enemy);
    }
    else if (shape == ProjectileShape::BEAM)
    {
        collision = CheckBeamProjectileVsEnemy(projectile, enemy);
    }
    // ヒット処理
    if (collision)
    {
        if (shape == ProjectileShape::BEAM)
        {
            // ビームは常時ヒット扱い
            OnProjectileHitEnemy(projectile, enemy);
        }
        else
        {
            // 通常弾は初回ヒットのみ処理
            if (!wasProjectileEnemyColliding)
            {
                OnProjectileHitEnemy(projectile, enemy);
            }
        }
    }
    wasProjectileEnemyColliding = collision;
    return collision;
}

bool CollisionManager::CheckEnemyWeaponPlayerCollision(Enemy& enemy, Player& player)
{
    // 武器が有効な時だけ判定
    if (!enemy.IsWeaponHitActive())
    {
        enemyWeaponPlayerColliding = false;
        return false;
    }
    // 武器カプセル取得
    DirectX::XMFLOAT3 weaponBase, weaponTip;
    float weaponRadius;
    if (!enemy.GetWeaponCapsule(weaponBase, weaponTip, weaponRadius))
    {
        enemyWeaponPlayerColliding = false;
        return false;
    }
    // プレイヤーカプセル取得
    DirectX::XMFLOAT3 playerBase, playerTip;
    player.GetBodyCapsuleEndpoints(playerBase, playerTip);
    float playerRadius = player.GetScaledBodyRadius();
    DirectX::XMFLOAT3 outPosition;
    // カプセル同士の判定
    enemyWeaponPlayerColliding = Collision::IntersectCapsuleVsCapsule(
        weaponBase, weaponTip, weaponRadius,
        playerBase, playerTip, playerRadius,
        outPosition
    );
    // デバッグ表示用カラー変更
    if (enemyWeaponPlayerColliding)
    {
        auto weaponParams = enemy.GetWeaponCapsuleParams();
        weaponParams.color = { 1.0f, 1.0f, 0.0f, 1.0f };
    }
    else
    {
        auto weaponParams = enemy.GetWeaponCapsuleParams();
        weaponParams.color = { 1.0f, 0.0f, 0.0f, 1.0f };
    }
    wasEnemyWeaponPlayerColliding = enemyWeaponPlayerColliding;
    return enemyWeaponPlayerColliding;
}

void CollisionManager::OnProjectileHitEnemy(Projectile& projectile, Enemy& enemy)
{
    // 弾を削除対象にする
    projectile.MarkForRemoval();
    // スタッフ弾の描画停止
    if (projectile.GetProjectileType() == "staff")
    {
        ParticleManager::Instance().StopStaffProjectileRendering();
    }
    // ヒット処理をヒット管理に渡す
    auto& hitManager = HitManager::Instance();
    hitManager.ProcessProjectileHit(projectile, enemy);
}

bool CollisionManager::CheckCapsuleProjectileVsEnemy(Projectile& projectile, Enemy& enemy)
{
    // 弾カプセル取得
    DirectX::XMFLOAT3 projBase, projTip;
    GetProjectileCapsuleEndpoints(projectile, projBase, projTip);
    float projRadius = projectile.GetRadius();
    // エネミーカプセル取得
    DirectX::XMFLOAT3 enemyBase, enemyTip;
    enemy.GetBodyCapsuleEndpoints(enemyBase, enemyTip);
    float enemyRadius = enemy.GetScaledBodyRadius();
    DirectX::XMFLOAT3 outEnemyPosition;
    // カプセル同士判定
    return Collision::IntersectCapsuleVsCapsule(
        projBase, projTip, projRadius,
        enemyBase, enemyTip, enemyRadius,
        outEnemyPosition
    );
}

bool CollisionManager::CheckBeamProjectileVsEnemy(Projectile& projectile, Enemy& enemy)
{
    // ビームの開始と終了点
    DirectX::XMFLOAT3 beamStart = projectile.GetBeamSpawnPoint();
    DirectX::XMFLOAT3 beamEnd = projectile.GetBeamTarget();
    // エネミーカプセル取得
    DirectX::XMFLOAT3 enemyBase, enemyTip;
    enemy.GetBodyCapsuleEndpoints(enemyBase, enemyTip);
    float enemyRadius = enemy.GetScaledBodyRadius();
    DirectX::XMFLOAT3 outEnemyPosition;
    // ビーム判定
    return Collision::IntersectCapsuleVsCapsule(
        beamStart, beamEnd, projectile.GetRadius(),
        enemyBase, enemyTip, enemyRadius,
        outEnemyPosition
    );
}

void CollisionManager::GetProjectileCapsuleEndpoints(
    Projectile& projectile,
    DirectX::XMFLOAT3& outBase,
    DirectX::XMFLOAT3& outTip)
{
    DirectX::XMFLOAT3 pos = projectile.GetPosition();
    // 方向取得
    DirectX::XMFLOAT3 dir = projectile.GetOrientation();
    // 方向が無い場合は移動方向
    if (dir.x == 0.0f && dir.y == 0.0f && dir.z == 0.0f)
    {
        dir = projectile.GetDirection();
    }
    float length = projectile.GetCapsuleLength();
    float half = length * 0.5f;
    // カプセルの上下
    outBase = {
        pos.x - dir.x * half,
        pos.y - dir.y * half,
        pos.z - dir.z * half
    };
    outTip = {
        pos.x + dir.x * half,
        pos.y + dir.y * half,
        pos.z + dir.z * half
    };
}

bool CollisionManager::CheckProjectilePlayerCollision(Projectile& projectile, Player& player)
{
    // 特殊攻撃は毎フレーム判定リセット
    if (projectile.GetProjectileType() == "berserk_meteor_impact" ||
        projectile.GetProjectileType() == "berserk_danger_zone_hit")
    {
        wasProjectilePlayerColliding = false;
    }
    // 弾の形状取得
    ProjectileShape shape = projectile.GetShape();
    // プレイヤーカプセル取得
    DirectX::XMFLOAT3 playerBase, playerTip;
    player.GetBodyCapsuleEndpoints(playerBase, playerTip);
    float playerRadius = player.GetScaledBodyRadius();
    bool collision = false;
    DirectX::XMFLOAT3 outPlayerPosition;
    // 弾の種類ごとに判定
    if (shape == ProjectileShape::SPHERE)
    {
        collision = Collision::IntersectSphereVsCapsule(
            projectile.GetPosition(), projectile.GetRadius(),
            playerBase, playerTip, playerRadius,
            outPlayerPosition
        );
    }
    else if (shape == ProjectileShape::CAPSULE)
    {
        DirectX::XMFLOAT3 projBase, projTip;
        GetProjectileCapsuleEndpoints(projectile, projBase, projTip);
        collision = Collision::IntersectCapsuleVsCapsule(
            projBase, projTip, projectile.GetRadius(),
            playerBase, playerTip, playerRadius,
            outPlayerPosition
        );
    }
    else if (shape == ProjectileShape::BEAM)
    {
        DirectX::XMFLOAT3 beamStart = projectile.GetBeamSpawnPoint();
        DirectX::XMFLOAT3 beamEnd = projectile.GetBeamTarget();
        collision = Collision::IntersectCapsuleVsCapsule(
            beamStart, beamEnd, projectile.GetRadius(),
            playerBase, playerTip, playerRadius,
            outPlayerPosition
        );
    }
    // 新規ヒット判定
    if (collision && !wasProjectilePlayerColliding)
    {
        OnProjectileHitPlayer(projectile, player);
    }
    wasProjectilePlayerColliding = collision;
    projectilePlayerColliding = collision;
    return collision;
}

void CollisionManager::OnProjectileHitPlayer(Projectile& projectile, Player& player)
{
    // 弾削除
    projectile.MarkForRemoval();
    // ヒット処理を管理へ渡す
    auto& hitManager = HitManager::Instance();
    hitManager.ProcessEnemyProjectileHit(projectile, player);
}