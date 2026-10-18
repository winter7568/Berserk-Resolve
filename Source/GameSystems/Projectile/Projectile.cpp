#include "Projectile.h"
#include "ProjectileManager.h"
#include "GameLib/Graphics.h"
#include "GameLib/ShapeRenderer.h"
#include "GameSystems/Debug/DebugRenderer.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameSystems/Particle/ParticleManager.h"
#include "GameSystems/Effect/EffectManager.h"

Projectile::Projectile(
    const std::string& ownerType,
    const std::string& projectileType,
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& direction)
    : base({ ownerType, projectileType, position, direction })
{
    // 設定データを取得
    config = ProjectileDataManager::Instance().GetConfig(projectileType);

    // 方向ベクトルを正規化
    float length = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (length > 0.001f) {
        base.direction.x = direction.x / length;
        base.direction.y = direction.y / length;
        base.direction.z = direction.z / length;
    }

    // モデルを設定
    ID3D11Device* device = Graphics::Instance().GetDevice();

    if (ownerType == "player" && projectileType == "pistol") {
        base.model = ProjectileManager::Instance().GetProjectileModel("player_bullet");
        base.scale = { 0.3f, 0.3f, 0.3f };
    }
    else if (ownerType == "enemy" && projectileType == "enemy_orb") {
        base.model = ProjectileManager::Instance().GetProjectileModel("enemy_orb");
        base.scale = { 0.3f, 0.3f, 0.3f };
        base.model->emissiveStrengthMultiplier = 2.5f;
    }

    if (projectileType == "air_slash") {
        base.model = ProjectileManager::Instance().GetProjectileModel("air_slash");
        base.scale = { 1.0f, 1.0f, 1.0f };
    }
    else if (projectileType == "bow_charged") {
        base.model = ProjectileManager::Instance().GetProjectileModel("bow_charged");
        base.scale = { 1.5f, 1.5f, 6.0f };
        base.model->emissiveStrengthMultiplier = 1.5f;
    }
}

void Projectile::Update(float elapsedTime)
{
    if (!base.isActive) return;

    // 通常の移動
    float speed = config.speed * elapsedTime;
    base.position.x += base.direction.x * speed;
    base.position.y += base.direction.y * speed;
    base.position.z += base.direction.z * speed;

    // ビームの特別処理
    if (config.isBeam && beamStayConnected) {
        base.lifetime += elapsedTime;

        if (base.lifetime > ProjectileConstants::BEAM_LIFETIME) {
            MarkForRemoval();
            return;
        }
    }
    else {
        base.lifetime += elapsedTime;

        // 寿命チェック
        if (base.lifetime > ProjectileConstants::MAX_PROJECTILE_LIFETIME) {
            MarkForRemoval();
            return;
        }

        // 移動距離チェック
        float distanceTraveled = config.speed * base.lifetime;
        if (distanceTraveled > ProjectileConstants::MAX_TRAVEL_DISTANCE) {
            MarkForRemoval();
            return;
        }
    }

    // 特定弾の寿命制御
    if (base.projectileType == "berserk_meteor_impact") {
        if (base.lifetime > ProjectileConstants::METEOR_IMPACT_LIFETIME) {
            MarkForRemoval();
            return;
        }
    }

    if (base.projectileType == "berserk_danger_zone_hit") {
        if (base.lifetime > ProjectileConstants::DANGER_ZONE_LIFETIME) {
            MarkForRemoval();
            return;
        }
    }

    // スラッシュのエフェクト
    if (base.projectileType == "air_slash") {
        // 軌跡エフェクト
        ParticleManager::Instance().SpawnSlashTrail(
            base.position, base.direction,
            base.orientation, config.capsuleLength, base.lifetime);

        // 一定間隔で光エフェクト
        edgeGlowTimer += elapsedTime;
        if (edgeGlowTimer >= ProjectileConstants::SLASH_EDGE_GLOW_INTERVAL) {
            ParticleManager::Instance().SpawnSlashEdgeGlow(
                base.position,
                base.orientation,
                config.capsuleLength
            );
            edgeGlowTimer = 0.0f;
        }
    }

    // 弓チャージの電気エフェクト
    if (base.projectileType == "bow_charged") {
        ParticleManager::Instance().SpawnElectricityEffect(
            base.position,
            base.direction,
            ProjectileConstants::BOW_ELECTRIC_SCALE,
            ProjectileConstants::BOW_ELECTRIC_INTENSITY
        );
    }

    // ホーミング処理
    if (config.homing && config.homingStrength > 0.0f) {
        ApplyHoming(elapsedTime);
    }

    // 行列更新
    base.UpdateTransform();

    // 弾の軌跡エフェクト
    if (base.projectileType == "pistol" && base.ownerType == "player") {
        ParticleManager::Instance().SpawnPlayerBulletTrail(
            base.position, base.direction, base.lifetime);
    }

    // 視覚エフェクトの生成と更新
    if (base.projectileType == "bow_quick") {
        if (!effectSpawned) {
            visualEffectHandle = EffectManager::Instance().PlayEffectWithHandle(
                "playerSpearArrow",
                base.position,
                ProjectileConstants::EFFECT_SCALE
            );
            effectSpawned = true;
        }
        else if (visualEffectHandle != -1) {
            EffectManager::Instance().SetEffectPosition(
                visualEffectHandle, base.position);
        }
    }
}

void Projectile::Render(const RenderContext& rc)
{
    if (!base.isActive) return;

    // モデル描画
    if (base.model) {
        ApplyRotationForProjectileType();

        ID3D11DeviceContext* dc = rc.deviceContext;
        Shader* shader = Graphics::Instance().GetShader(ShaderId::PBR);

        // 加算合成で描画するタイプ
        if (base.projectileType == "pistol" || base.projectileType == "air_slash")
        {
            shader->Begin(rc);

            const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Additive), blendFactor, 0xFFFFFFFF);

            shader->Draw(rc, base.model.get());

            shader->End(rc);

            dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Opaque), blendFactor, 0xFFFFFFFF);
        }
        // 通常描画
        else if (base.projectileType == "bow_charged" || base.projectileType == "enemy_orb")
        {
            shader->Begin(rc);
            shader->Draw(rc, base.model.get());
            shader->End(rc);
        }
    }
}

void Projectile::ApplyRotationForProjectileType()
{
    // air_slash の回転処理
    if (base.projectileType == "air_slash") {
        DirectX::XMFLOAT3 dir = base.direction;

        float horizontalLen = sqrt(dir.x * dir.x + dir.z * dir.z);
        float pitch = atan2f(-dir.y, horizontalLen);
        float yaw = atan2f(dir.x, dir.z);

        DirectX::XMMATRIX rotationY = DirectX::XMMatrixRotationY(yaw);
        DirectX::XMMATRIX rotationX = DirectX::XMMatrixRotationX(pitch);
        DirectX::XMMATRIX rotation = rotationX * rotationY;

        DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(base.scale.x, base.scale.y, base.scale.z);
        DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(base.position.x, base.position.y, base.position.z);
        DirectX::XMMATRIX world = scale * rotation * translation;

        DirectX::XMFLOAT4X4 worldTransform;
        DirectX::XMStoreFloat4x4(&worldTransform, world);
        base.model->UpdateTransform(worldTransform);
        return;
    }

    // 弓チャージ弾
    if (base.projectileType == "bow_charged") {
        float angleY = atan2f(base.direction.x, base.direction.z);

        DirectX::XMMATRIX rotationY = DirectX::XMMatrixRotationY(angleY);
        DirectX::XMMATRIX rotationX = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(ProjectileConstants::MODEL_BAKED_X_ROTATION_DEG));
        DirectX::XMMATRIX rotation = rotationX * rotationY;

        DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(base.scale.x, base.scale.y, base.scale.z);
        DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(base.position.x, base.position.y, base.position.z);
        DirectX::XMMATRIX world = scale * rotation * translation;

        DirectX::XMFLOAT4X4 worldTransform;
        DirectX::XMStoreFloat4x4(&worldTransform, world);
        base.model->UpdateTransform(worldTransform);
        return;
    }

    // 敵の弾
    if (base.projectileType == "enemy_orb") {
        float angleY = atan2f(base.direction.x, base.direction.z);

        DirectX::XMMATRIX rotationY = DirectX::XMMatrixRotationY(angleY);
        DirectX::XMMATRIX rotationX = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(ProjectileConstants::MODEL_BAKED_X_ROTATION_DEG));
        DirectX::XMMATRIX rotation = rotationX * rotationY;

        DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(base.scale.x, base.scale.y, base.scale.z);
        DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(base.position.x, base.position.y, base.position.z);
        DirectX::XMMATRIX world = scale * rotation * translation;

        DirectX::XMFLOAT4X4 worldTransform;
        DirectX::XMStoreFloat4x4(&worldTransform, world);
        base.model->UpdateTransform(worldTransform);
        return;
    }

    // それ以外は通常の行列を使用
    base.model->UpdateTransform(base.worldTransform);
}

void Projectile::MarkForRemoval()
{
    if (visualEffectHandle != -1) {
        EffectManager::Instance().StopEffect(visualEffectHandle);
        visualEffectHandle = -1;
    }

    base.isActive = false;
}

void Projectile::ApplyHoming(float elapsedTime)
{
    // 敵とプレイヤーの位置取得
    DirectX::XMFLOAT3 enemyPos = Enemy::Instance().GetPosition();
    DirectX::XMFLOAT3 enemyOffset = { 0.0f, 8.0f, 0.0f };
    DirectX::XMFLOAT3 playerPos = Player::Instance().GetPosition();

    // 弾と敵の距離計算
    float dx = enemyPos.x - base.position.x;
    float dy = (enemyPos.y + enemyOffset.y) - base.position.y;
    float dz = enemyPos.z - base.position.z;
    float projectileToEnemyDistance = sqrt(dx * dx + dy * dy + dz * dz);

    if (projectileToEnemyDistance < ProjectileConstants::MIN_HOMING_DISTANCE) {
        return;
    }

    // 敵への方向ベクトル
    DirectX::XMFLOAT3 toEnemy = {
        enemyPos.x - base.position.x,
        enemyPos.y - base.position.y + enemyOffset.y,
        enemyPos.z - base.position.z
    };

    float length = sqrt(toEnemy.x * toEnemy.x + toEnemy.y * toEnemy.y + toEnemy.z * toEnemy.z);
    if (length < 0.001f) return;

    toEnemy.x /= length;
    toEnemy.y /= length;
    toEnemy.z /= length;

    // プレイヤーの前方向
    const DirectX::XMFLOAT3 playerForward = Player::Instance().GetForwardVector();

    // プレイヤーから敵への方向
    DirectX::XMFLOAT3 playerToEnemy = {
        enemyPos.x - playerPos.x,
        enemyPos.y - playerPos.y,
        enemyPos.z - playerPos.z
    };

    float playerToEnemyLen = sqrt(
        playerToEnemy.x * playerToEnemy.x +
        playerToEnemy.y * playerToEnemy.y +
        playerToEnemy.z * playerToEnemy.z
    );

    if (playerToEnemyLen < 0.001f) return;

    playerToEnemy.x /= playerToEnemyLen;
    playerToEnemy.y /= playerToEnemyLen;
    playerToEnemy.z /= playerToEnemyLen;

    // 視野内チェック
    float visionDot =
        playerForward.x * playerToEnemy.x +
        playerForward.y * playerToEnemy.y +
        playerForward.z * playerToEnemy.z;

    const float fovCosine = cosf(DirectX::XMConvertToRadians(ProjectileConstants::FOV_CONE_ANGLE));

    if (visionDot < fovCosine) {
        return;
    }

    // 現在の方向
    DirectX::XMFLOAT3 currentDir = base.direction;

    // 角度計算
    float dot = currentDir.x * toEnemy.x + currentDir.y * toEnemy.y + currentDir.z * toEnemy.z;
    dot = max(-1.0f, min(1.0f, dot));
    float angle = acosf(dot);

    // 最大回転量
    float maxTurn = config.maxHomingAngle * config.homingStrength * elapsedTime * ProjectileConstants::HOMING_FRAMERATE;
    if (angle > maxTurn)
        angle = maxTurn;

    if (angle < 0.001f) return;

    // 回転軸を計算
    DirectX::XMFLOAT3 axis = {
        currentDir.y * toEnemy.z - currentDir.z * toEnemy.y,
        currentDir.z * toEnemy.x - currentDir.x * toEnemy.z,
        currentDir.x * toEnemy.y - currentDir.y * toEnemy.x
    };

    float axisLen = sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
    if (axisLen < 0.001f) return;

    axis.x /= axisLen;
    axis.y /= axisLen;
    axis.z /= axisLen;

    // 回転計算
    float cosAngle = cosf(angle);
    float sinAngle = sinf(angle);

    DirectX::XMFLOAT3 newDir = {
        currentDir.x * cosAngle + (axis.y * currentDir.z - axis.z * currentDir.y) * sinAngle,
        currentDir.y * cosAngle + (axis.z * currentDir.x - axis.x * currentDir.z) * sinAngle,
        currentDir.z * cosAngle + (axis.x * currentDir.y - axis.y * currentDir.x) * sinAngle
    };

    base.direction = newDir;
}

void Projectile::GetCapsuleEndpointsForRendering(DirectX::XMFLOAT3& outBase, DirectX::XMFLOAT3& outTip)
{
    if (config.isBeam && beamStayConnected) {
        outBase = beamSpawnPoint;
        outTip = beamTarget;
        return;
    }

    float capsuleLength = config.capsuleLength;
    float halfLength = capsuleLength * 0.5f;

    DirectX::XMFLOAT3 useDirection = base.direction;
    if (base.orientation.x != 0.0f || base.orientation.y != 0.0f || base.orientation.z != 0.0f) {
        useDirection = base.orientation;
    }

    outBase = {
        base.position.x - useDirection.x * halfLength,
        base.position.y - useDirection.y * halfLength,
        base.position.z - useDirection.z * halfLength
    };

    outTip = {
        base.position.x + useDirection.x * halfLength,
        base.position.y + useDirection.y * halfLength,
        base.position.z + useDirection.z * halfLength
    };
}