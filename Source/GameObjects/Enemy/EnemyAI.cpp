#include "Enemy.h"
#include "GameObjects/Player/Player.h"
#include "StateMachine/EnemyStates/PositioningState.h"

// ============================================================================
// PUBLIC METHODS - AI & Decision Making
// ============================================================================

bool Enemy::IsFacingPlayer() const
{
    Player& player = Player::Instance();

    // プレイヤーへの方向ベクトル（高さは無視）
    DirectX::XMFLOAT3 toPlayer = {
        player.GetPosition().x - GetPosition().x,
        0.0f,
        player.GetPosition().z - GetPosition().z
    };

    // ベクトルを正規化する（長さを1にする）
    float length = sqrt(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);
    if (length > CharacterConstants::VECTOR_LENGTH_EPSILON) {
        toPlayer.x /= length;
        toPlayer.z /= length;
    }

    // 敵がプレイヤーの方向を向いているかチェック（内積）
    DirectX::XMFLOAT3 enemyForward = GetForwardVector();
    float dot = toPlayer.x * enemyForward.x + toPlayer.z * enemyForward.z;

    // 約60度以内なら正面を向いている
    return dot > EnemyConstants::FACING_DOT_THRESHOLD;
}

float Enemy::CalculateDistanceToPlayer() const
{
    Player& player = Player::Instance();

    DirectX::XMFLOAT3 playerPos = player.GetPosition();
    DirectX::XMFLOAT3 enemyPos = GetPosition();

    // XZ平面での距離を計算（高さは無視）
    float dx = playerPos.x - enemyPos.x;
    float dz = playerPos.z - enemyPos.z;

    return sqrt(dx * dx + dz * dz);
}

DirectX::XMFLOAT2 Enemy::GetDirectionToPlayer() const
{
    Player& player = Player::Instance();
    DirectX::XMFLOAT3 playerPos = player.GetPosition();
    DirectX::XMFLOAT3 enemyPos = GetPosition();

    float dirX = playerPos.x - enemyPos.x;
    float dirZ = playerPos.z - enemyPos.z;
    float distance = std::sqrt(dirX * dirX + dirZ * dirZ);

    if (distance > 0.01f)
    {
        return { dirX / distance, dirZ / distance };
    }

    return { 0.0f, 0.0f };
}

void Enemy::SmoothlyTrackPlayer(float elapsedTime, float turnSpeed)
{
    DirectX::XMFLOAT2 dir = GetDirectionToPlayer();

    if (dir.x != 0.0f || dir.y != 0.0f)
    {
        Turn(elapsedTime, dir.x, dir.y, turnSpeed);
    }
}

bool Enemy::ShouldBlock(float distanceToPlayer, bool isPlayerAttacking) const
{
    if (!isPlayerAttacking) return false;
    if (distanceToPlayer > EnemyConstants::BLOCK_MAX_DISTANCE) return false;

    float blockChance = GetPersonalityBlockChance();

    // 距離が近いほどブロックしやすい（距離0で1.0、距離MAXで0.0）
    float distanceFactor = 1.0f - (distanceToPlayer / EnemyConstants::BLOCK_MAX_DISTANCE);

    // ブロック確率 = 基本確率 × (最小倍率 + 範囲 × 距離係数)
    float adjustedChance = blockChance * (EnemyConstants::BLOCK_MIN_MULTIPLIER + EnemyConstants::BLOCK_MULTIPLIER_RANGE * distanceFactor);

    return (rand() % CharacterConstants::PERCENTAGE_BASE) < (adjustedChance * CharacterConstants::PERCENTAGE_BASE);
}

bool Enemy::ShouldDash(float distanceToPlayer) const
{
    if (distanceToPlayer > EnemyConstants::DASH_MAX_DISTANCE) return false;

    float dashChance = GetPersonalityDashChance();

    // 距離が近いほどダッシュしやすい
    float distanceFactor = 1.0f - (distanceToPlayer / EnemyConstants::DASH_MAX_DISTANCE);
    float adjustedChance = dashChance * (EnemyConstants::DASH_MIN_MULTIPLIER + EnemyConstants::DASH_MULTIPLIER_RANGE * distanceFactor);

    return (rand() % CharacterConstants::PERCENTAGE_BASE) < (adjustedChance * CharacterConstants::PERCENTAGE_BASE);
}

bool Enemy::ShouldCounterAfterBlock() const
{
    float counterChance = GetPersonalityCounterChance();
    return (rand() % CharacterConstants::PERCENTAGE_BASE) < (counterChance * CharacterConstants::PERCENTAGE_BASE);
}

bool Enemy::ShouldUseSpecialAttack() const
{
    float specialChance = GetPersonalitySpecialAttackChance();
    return (rand() % CharacterConstants::PERCENTAGE_BASE) < (specialChance * CharacterConstants::PERCENTAGE_BASE);
}

bool Enemy::ShouldTeleport() const
{
    float teleportChance = GetPersonalityTeleportChance();
    return (rand() % CharacterConstants::PERCENTAGE_BASE) < (teleportChance * CharacterConstants::PERCENTAGE_BASE);
}

float Enemy::GetPersonalityBlockChance() const
{
    return GetPositioningBrain().GetBlockChance();
}

float Enemy::GetPersonalityDashChance() const
{
    return GetPositioningBrain().GetDashChance();
}

float Enemy::GetPersonalityCounterChance() const
{
    return GetPositioningBrain().GetCounterChance();
}

float Enemy::GetPersonalitySpecialAttackChance() const
{
    return GetPositioningBrain().GetSpecialAttackChance();
}

float Enemy::GetPersonalityTeleportChance() const
{
    return GetPositioningBrain().GetTeleportChance();
}